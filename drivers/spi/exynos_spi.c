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
#include <malloc.h>
#include <spi.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-exynos/spi.h>
#include <asm/io.h>

struct exynos_spi_slave {
	struct spi_slave slave;
	struct exynos_spi *regs;
	unsigned int freq;
	unsigned int mode;
	enum periph_id periph_id;	/* Peripheral ID for this device */
};

/* We should not rely on any particular ordering of these IDs */
static enum periph_id periph_for_dev[] = {
	PERIPH_ID_SPI0,
	PERIPH_ID_SPI1,
	PERIPH_ID_SPI2,
};

static inline struct exynos_spi_slave *to_exynos_spi(struct spi_slave *slave)
{
	return container_of(slave, struct exynos_spi_slave, slave);
}

static inline struct exynos_spi *exynos_get_base_spi(int dev_index)
{
	return (struct exynos_spi *)samsung_get_base_spi() + dev_index;
}

static enum periph_id spi_get_periph_id(unsigned dev_index)
{
	if (dev_index < ARRAY_SIZE(periph_for_dev))
		return periph_for_dev[dev_index];
	debug("%s: invalid bus %d", __func__, dev_index);
	return PERIPH_ID_NONE;
}

void spi_init()
{
	/* do nothing */
}

/**
 * Set the required clock frequency to SPI controller
 *
 * @param spi_slave	SPI controller
 * @param clk		Base address of clock register map
 */
static void spi_set_clk(struct exynos_spi_slave *spi_slave,
			struct exynos5_clock *clk)
{
	uint dev_index = spi_slave->slave.bus, div = 0;
	u32 addr;
	u32 val;

	if (dev_index == 0 || dev_index == 1)
		addr = (u32)&clk->div_peric1;
	else {
		addr = (u32)&clk->div_peric2;
		dev_index -= 2;
	}

	if (!(spi_slave->mode & SPI_SLAVE)) {
		if (spi_slave->freq > EXYNOS_SPI_MAX_FREQ)
			spi_slave->freq = EXYNOS_SPI_MAX_FREQ;

		div = EXYNOS_SPI_MAX_FREQ / spi_slave->freq - 1;
	}

	val = readl(addr);
	val &= ~(0xff << ((dev_index << 4) + 8));
	val |= (div & 0xff) << ((dev_index << 4) + 8);
	writel(val, addr);
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
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct exynos_spi_slave *spi_slave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	spi_slave = malloc(sizeof(*spi_slave));
	if (!spi_slave)
		return NULL;

	spi_slave->slave.bus = bus;
	spi_slave->slave.cs = cs;
	spi_slave->regs = exynos_get_base_spi(bus);
	spi_slave->mode = mode;
	spi_slave->periph_id = spi_get_periph_id(bus);

	if (max_hz > EXYNOS_SPI_MAX_FREQ)
		spi_slave->freq = EXYNOS_SPI_MAX_FREQ;
	else
		spi_slave->freq = max_hz;

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
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	u32 reg = 0;
	int ret;

	spi_set_clk(spi_slave, clk);
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
			debug("txp:0x%x\n", txp[i]);
			do {
				sts = readl(&regs->spi_sts);
			} while (!((sts >> SPI_RX_LVL_OFFSET) &
				fifo_lvl_mask));
			data = readl(&regs->rx_data);
			if (din)
				*rxp++ = data;
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
	return bus < 3 && cs == 0;
}

/**
 * Get the chip select gpio number and base address of gpio bank
 *
 * @param spi_slave	SPI controller
 * @param reg		Pointer to hold the required gpio bank address
 * @return the cs gpio based on spi bus number
 */
static int spi_get_cs(struct exynos_spi_slave *spi_slave,
		      struct s5p_gpio_bank **reg)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	if (spi_slave->slave.bus == 0) {
		*reg = &gpio1->a2;
		return 1;
	} else if (spi_slave->slave.bus == 1) {
		*reg = &gpio1->a2;
		return 5;
	} else if (spi_slave->slave.bus == 2) {
		*reg = &gpio1->b1;
		return 2;
	} else {
		debug("%s invalid bus:%d\n", __func__, spi_slave->slave.bus);
		return 0;
	}
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
	struct s5p_gpio_bank *reg = 0;
	int cs_line;

	cs_line = spi_get_cs(spi_slave, &reg);
	if (reg) {
		s5p_gpio_direction_output(reg, cs_line, 1);
		s5p_gpio_cfg_pin(reg, cs_line, GPIO_FUNC(0x1));
		s5p_gpio_set_pull(reg, cs_line, GPIO_PULL_UP);
		s5p_gpio_set_value(reg, cs_line, 0);
		debug("Activate CS:%d\n", s5p_gpio_get_value(reg, cs_line));
	} else
		debug("Invalid CS Reg\n");
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
	struct s5p_gpio_bank *reg = 0;
	int cs_line;

	cs_line = spi_get_cs(spi_slave, &reg);
	if (reg) {
		s5p_gpio_set_value(reg, cs_line, 1);
		debug("Deactivate CS:%d\n",
		       s5p_gpio_get_value(reg, cs_line));
	} else
		debug("Invalid CS Reg\n");
}
