/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Padmavathi Venna <padma.v@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <spi.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-exynos/spi.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* Information about each SPI controller */
struct spi_bus {
	enum periph_id periph_id;
	int node;		/* Device tree node */
	s32 frequency;		/* Default clock frequency, -1 for none */
	struct exynos_spi *regs;
	int inited;		/* 1 if this bus is ready for use */
};

/* A list of spi buses that we know about */
static struct spi_bus spi_bus[EXYNOS5_SPI_NUM_CONTROLLERS];
static unsigned int bus_count;

struct exynos_spi_slave {
	struct spi_slave slave;
	struct exynos_spi *regs;
	unsigned int freq;		/* Default frequency */
	unsigned int mode;
	enum periph_id periph_id;	/* Peripheral ID for this device */
};

static struct spi_bus *spi_get_bus(unsigned dev_index)
{
	if (dev_index < bus_count)
		return &spi_bus[dev_index];
	debug("%s: invalid bus %d", __func__, dev_index);

	return NULL;
}

static inline struct exynos_spi_slave *to_exynos_spi(struct spi_slave *slave)
{
	return container_of(slave, struct exynos_spi_slave, slave);
}

/**
 * Set the required clock frequency to SPI controller
 *
 * @param spi_slave	SPI controller
 */
static void spi_set_clk(struct exynos_spi_slave *spi_slave)
{
	uint div = 0;

	if (!(spi_slave->mode & SPI_SLAVE)) {
		if (spi_slave->freq > EXYNOS_SPI_MAX_FREQ)
			spi_slave->freq = EXYNOS_SPI_MAX_FREQ;

		div = EXYNOS_SPI_MAX_FREQ / spi_slave->freq - 1;
	}

	clock_ll_set_pre_ratio(spi_slave->periph_id, div);
}

/**
 * Initialize the gpio for Master and Slave Mode
 *
 * @param spi_slave	SPI controller
 */
static void spi_pinmux_init(struct exynos_spi_slave *spi)
{
	int flags = PINMUX_FLAG_NONE;

	if (spi->mode & SPI_SLAVE)
		flags |= PINMUX_FLAG_SLAVE_MODE;

	exynos_pinmux_config(spi->periph_id, flags);
}

/**
 * Setup the driver private data
 *
 * @param bus		ID of the bus that the slave is attached to
 * @param cs		ID of the chip select connected to the slave
 * @param max_hz	Required spi frequency
 * @param mode		Required spi mode (clk polarity, clk phase and
 *			master or slave)
 * @return new device or NULL
 */
struct spi_slave *spi_setup_slave(unsigned int busnum, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct exynos_spi_slave *spi_slave;
	struct spi_bus *bus;

	if (!spi_cs_is_valid(busnum, cs)) {
		debug("%s: Invalid bus/chip select %d, %d\n", __func__,
		      busnum, cs);
		return NULL;
	}

	spi_slave = malloc(sizeof(*spi_slave));
	if (!spi_slave) {
		debug("%s: Could not allocate spi_slave\n", __func__);
		return NULL;
	}

	bus = &spi_bus[busnum];
	spi_slave->slave.bus = busnum;
	spi_slave->slave.cs = cs;
	spi_slave->regs = bus->regs;
	spi_slave->mode = mode;
	spi_slave->periph_id = bus->periph_id;

	spi_slave->freq = bus->frequency;
	if (max_hz)
		spi_slave->freq = min(max_hz, spi_slave->freq);

	return &spi_slave->slave;
}

/**
 * Free spi controller
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
void spi_free_slave(struct spi_slave *slave)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);

	free(spi_slave);
}

/**
 * Flush spi tx, rx fifos and reset the SPI controller
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 * @return zero on success else a negative value
 */
int spi_flush_fifo(struct spi_slave *slave)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);
	struct exynos_spi *regs = spi_slave->regs;
	unsigned int fifo_lvl_mask, fifo_lvl_offset;
	u32 sts;
	ulong start;

	writel(0, &regs->pkt_cnt);
	clrsetbits_le32(&regs->ch_cfg, SPI_CH_HS_EN, SPI_CH_RST);

	/* Flush TxFIFO and the RxFIFO */
	fifo_lvl_mask = spi_slave->slave.bus ?
		SPI_FIFO_LVL_MASK_CH_1_2 : SPI_FIFO_LVL_MASK_CH_0;
	fifo_lvl_offset = SPI_TX_LVL_OFFSET;
	start = get_timer(0);
	for (;;) {
		sts = readl(&regs->spi_sts);
		if ((sts >> fifo_lvl_offset) & fifo_lvl_mask) {
			if (fifo_lvl_offset == SPI_RX_LVL_OFFSET)
				readl(&regs->rx_data);
		} else {
			if (fifo_lvl_offset == SPI_RX_LVL_OFFSET)
				break;
			/* Reset timeout and move on to rx fifo */
			fifo_lvl_offset = SPI_RX_LVL_OFFSET;
			start = get_timer(0);
		}
		if (get_timer(start) > SPI_TIMEOUT_MS) {
			debug("Timeout in flushing tx/rx fifo\n");
			return -1;
		}
	}

	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_TX_CH_ON | SPI_RX_CH_ON);

	return 0;
}

/**
 * Initialize the spi base registers, set the required clock frequency and
 * initialize the gpios
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 * @return zero on success else a negative value
 */
int spi_claim_bus(struct spi_slave *slave)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);
	struct exynos_spi *regs = spi_slave->regs;
	u32 reg = 0;
	int ret;

	spi_set_clk(spi_slave);
	spi_pinmux_init(spi_slave);

	ret = spi_flush_fifo(slave);

	reg = readl(&regs->ch_cfg);
	reg &= ~(SPI_CH_CPHA_B | SPI_CH_CPOL_L | SPI_SLAVE_MODE);

	if (spi_slave->mode & SPI_CPHA)
		reg |= SPI_CH_CPHA_B;

	if (spi_slave->mode & SPI_CPOL)
		reg |= SPI_CH_CPOL_L;

	if (spi_slave->mode & SPI_SLAVE)
		reg |= SPI_SLAVE_MODE;

	writel(reg, &regs->ch_cfg);
	writel(SPI_FB_DELAY_180, &regs->fb_clk);

	return ret;
}

/**
 * Reset the spi H/W and flush the tx and rx fifos
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
void spi_release_bus(struct spi_slave *slave)
{
	spi_flush_fifo(slave);
}

/**
 * Transfer and receive data
 *
 * @param slave		Pointer to spi_slave to which controller has to
 *			communicate with
 * @param bitlen	No of bits to tranfer or receive
 * @param dout		Pointer to transfer buffer
 * @param din		Pointer to receive buffer
 * @param flags		Flags for transfer begin and end
 * @return zero on success else a negative value
 */
int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);
	struct exynos_spi *regs = spi_slave->regs;
	/* spi core configured to do 8 bit transfers */
	uint bytes = bitlen / 8;
	const uchar *txp = dout;
	uchar *rxp = din;
	unsigned int fifo_lvl_mask;
	u32 sts;
	uint i;
	uint fifo_lvl, fifo_shift_amount = 6;

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
			slave->bus, slave->cs, bitlen, bytes, flags);

	if (bitlen == 0)
		return -1;

	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		return -1;
	}

	fifo_lvl_mask = spi_slave->slave.bus ?
		SPI_FIFO_LVL_MASK_CH_1_2 : SPI_FIFO_LVL_MASK_CH_0;

	fifo_lvl = (fifo_lvl_mask >> 1) + 1;
	if (!slave->bus)
		fifo_shift_amount = 8;

	writel(bytes | SPI_PACKET_CNT_EN, &regs->pkt_cnt);

	if ((flags & SPI_XFER_BEGIN) && !(spi_slave->mode & SPI_SLAVE))
		spi_cs_activate(slave);

	clrbits_le32(&regs->cs_reg, SPI_SLAVE_SIG_INACT);

	if (dout) {
		setbits_le32(&regs->ch_cfg, SPI_TX_CH_ON | SPI_RX_CH_ON);
		for (i = 0; i < bytes; i++) {
			uchar data;

			writel(txp[i], &regs->tx_data);
			debug("txp:0x%x", txp[i]);
			do {
				sts = readl(&regs->spi_sts);
			} while (!((sts >> SPI_RX_LVL_OFFSET) &
				fifo_lvl_mask));
			data = readl(&regs->rx_data);
			if (din) {
				*rxp++ = data;
				debug(", rxp:0x%x", data);
			}
			debug("\n");
		}
		clrbits_le32(&regs->ch_cfg, SPI_TX_CH_ON);
	}

	if (din && !dout) {
		int no_loops = 1, no_pkts = fifo_lvl, j;

		if (bytes > fifo_lvl) {
			no_loops = bytes >> fifo_shift_amount;
			if (bytes & (fifo_lvl - 1))
				++no_loops;
		}

		for (i = 1; i <= no_loops; i++) {
			if ((i << fifo_shift_amount) > bytes)
				no_pkts = bytes & (fifo_lvl - 1);
			writel(no_pkts | SPI_PACKET_CNT_EN,
				&regs->pkt_cnt);
			setbits_le32(&regs->ch_cfg, SPI_RX_CH_ON);

			do {
				sts = readl(&regs->spi_sts);
			} while (((sts >> SPI_RX_LVL_OFFSET) & fifo_lvl_mask) <
					no_pkts);

			for (j = 0; j < no_pkts; j++)
				*rxp++ = readl(&regs->rx_data);

			spi_flush_fifo(slave);
		}
	}

	setbits_le32(&regs->cs_reg, SPI_SLAVE_SIG_INACT);
	spi_flush_fifo(slave);

	if ((flags & SPI_XFER_END) && !(spi_slave->mode & SPI_SLAVE))
		spi_cs_deactivate(slave);

	return 0;
}

/**
 * Validates the bus and chip select numbers
 *
 * @param bus	ID of the bus that the slave is attached to
 * @param cs	ID of the chip select connected to the slave
 * @return one on success else zero
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return spi_get_bus(bus) && cs == 0;
}

/**
 * Activate the CS by driving it LOW
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
void spi_cs_activate(struct spi_slave *slave)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);

	exynos_pinmux_config(spi_slave->periph_id,
			     PINMUX_FLAG_CS | PINMUX_FLAG_ACTIVATE);
	debug("Activate CS, bus %d\n", spi_slave->slave.bus);
}

/**
 * Deactivate the CS by driving it HIGH
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
void spi_cs_deactivate(struct spi_slave *slave)
{
	struct exynos_spi_slave *spi_slave = to_exynos_spi(slave);

	exynos_pinmux_config(spi_slave->periph_id, PINMUX_FLAG_CS);
	debug("Deactivate CS, bus %d\n", spi_slave->slave.bus);
}

/**
 * Read the SPI config from the device tree node.
 *
 * @param blob	FDT blob to read from
 * @param node	Node offset to read from
 * @param bus	SPI bus structure to fill with information
 * @return 0 if ok, or -FDT_ERR_NOTFOUND if something was missing
 */
static int spi_get_config(const void *blob, int node, struct spi_bus *bus)
{
	bus->node = node;
	bus->regs = (struct exynos_spi *)fdtdec_get_addr(blob, node, "reg");
	bus->periph_id = clock_decode_periph_id(blob, node);
	if (bus->periph_id == PERIPH_ID_NONE) {
		debug("%s: Invalid peripheral ID %d\n", __func__,
		      bus->periph_id);
		return -FDT_ERR_NOTFOUND;
	}

	/* Use 500KHz as a suitable default */
	bus->frequency = fdtdec_get_int(blob, node, "clock-frequency", 500000);

	return 0;
}

/**
 * Process a list of nodes, adding them to our list of SPI ports.
 *
 * @param blob		fdt blob
 * @param node_list	list of nodes to process (any <=0 are ignored)
 * @param count		number of nodes to process
 * @param is_dvc	1 if these are DVC ports, 0 if standard I2C
 * @return 0 if ok, -1 on error
 */
static int process_nodes(const void *blob, int node_list[], int count)
{
	int i;

	/* build the i2c_controllers[] for each controller */
	for (i = 0; i < count; i++) {
		int node = node_list[i];
		struct spi_bus *bus;

		if (node <= 0)
			continue;

		bus = &spi_bus[i];
		if (spi_get_config(blob, node, bus)) {
			printf("exynos spi_init: failed to decode bus %d\n",
			       i);
			return -1;
		}

		debug("spi: controller bus %d at %p, periph_id %d\n",
		      i, bus->regs, bus->periph_id);
		bus->inited = 1;
		bus_count++;
	}

	return 0;
}

/**
 * Set up a new SPI slave for an fdt node
 *
 * @param blob		Device tree blob
 * @param node		SPI peripheral node to use
 * @return 0 if ok, -1 on error
 */
struct spi_slave *spi_setup_slave_fdt(const void *blob, int node,
		unsigned int cs, unsigned int max_hz, unsigned int mode)
{
	struct spi_bus *bus;
	unsigned int i;

	for (i = 0, bus = spi_bus; i < bus_count; i++, bus++) {
		if (bus->node == node)
			return spi_setup_slave(i, cs, max_hz, mode);
	}

	debug("%s: Failed to find bus node %d\n", __func__, node);
	return NULL;
}

/* Sadly there is no error return from this function */
void spi_init(void)
{
	int node_list[EXYNOS5_SPI_NUM_CONTROLLERS];
	const void *blob = gd->fdt_blob;
	int count;

	count = fdtdec_find_aliases_for_id(blob, "spi",
			COMPAT_SAMSUNG_EXYNOS_SPI, node_list,
			EXYNOS5_SPI_NUM_CONTROLLERS);
	if (process_nodes(blob, node_list, count))
		return;
}
