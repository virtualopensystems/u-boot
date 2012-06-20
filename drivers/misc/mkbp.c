/*
 * Chromium OS mkbp driver
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * The Matrix Keyboard Protocol driver handles talking to the keyboard
 * controller chip. Mostly this is for keyboard functions, but some other
 * things have slipped in, so we provide generic services to talk to the
 * KBC.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <mkbp.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

/* Which interface is the device on? */
enum mkbp_interface_t {
	MKBPIF_NONE,
	MKBPIF_SPI,
	MKBPIF_I2C,
	MKBPIF_LPC,	/* Intel Low Pin Count interface */
};

/* Our configuration information */
struct mkbp_dev {
	enum mkbp_interface_t interface;
	struct spi_slave *spi;		/* Our SPI slave, if using SPI */
	int parent_node;		/* Our parent node (interface) */
	unsigned int cs;		/* Our chip select */
	unsigned int addr;		/* Device address (for I2C) */
	unsigned int bus_num;		/* Bus number (for I2C) */
	unsigned int max_frequency;	/* Maximum interface frequency */
	struct fdt_gpio_state ec_int;	/* GPIO used as EC interrupt line */
	int lpc_cmd;			/* LPC command IO port */
	int lpc_data;			/* LPC command IO port */
	int lpc_param;			/* LPC param IO port */
	int lpc_param_len;		/* Length of LPC param space */
	uint8_t din[MSG_BYTES];		/* Input data buffer */
	uint8_t dout[MSG_BYTES];	/* Output data buffer */
};

static struct mkbp_dev static_dev, *last_dev;

#ifdef CONFIG_MKBP_I2C
static int mkbp_i2c_message(struct mkbp_dev *dev, uint8_t din[], int din_len,
			     uint8_t dout[])
{
	int old_bus = 0;

	old_bus = i2c_get_bus_num();
	/* Set to the proper i2c bus */
	if (i2c_set_bus_num(dev->bus_num)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			dev->bus_num);
		return -1;
	}
	/* Send read key state command */
	if (i2c_write(dev->addr, 0, 0, dout, 1)) {
		debug("%s: Cannot complete I2C write to 0x%x\n",
			__func__, dev->addr);
		return -1;
	}
	/* Receive the key state */
	if (i2c_read(dev->addr, 0, 0, din, din_len)) {
		debug("%s: Cannot complete I2C read from 0x%x\n",
			__func__, dev->addr);
		return -1;
	}
	/* Return to original bus number */
	if (i2c_set_bus_num(old_bus)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			old_bus);
		return -1;
	}
	/*
	 * I2C currently uses a simpler protocol, so we don't
	 * deal with the full header, the checksum byte on the end
	 * is ignored.
	 * TODO(bhthompson): Migrate to new protocol.
	 */
	if (din[0] != 0x00) /* EC_RES_SUCCESS */
		return -1;

	return 0;
}
#endif

#ifdef CONFIG_MKBP_LPC
static int wait_for_sync(struct mkbp_dev *dev)
{
	unsigned long start;

	start = get_timer(0);
	while (inb(dev->lpc_cmd) & EC_LPC_STATUS_BUSY_MASK) {
		if (get_timer(start) > 1000) {
			debug("%s: Timeout waiting for MKBP sync\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int mkbp_lpc_message(struct mkbp_dev *dev, uint8_t din[], int din_len,
			     uint8_t dout[])
{
	int ret, i;

	if (din_len > dev->lpc_param_len) {
		debug("%s: Cannot send %d bytes\n", __func__, din_len);
		return -1;
	}

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return 01;
	}

	debug_trace("cmd: %02x ", dout[0]);
	for (i = 1; i < din_len; i++) {
		debug_trace("%02x ", dout[i]);
		outb(dout[i], dev->lpc_param - 1 + i);
	}
	outb(dout[0], dev->lpc_cmd);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return 01;
	}

	ret = inb(dev->lpc_data);
	if (ret) {
		debug("%s: MKBP result code %d\n", __func__, ret);
		return -1;
	}

	din[0] = ret;
	debug_trace("resp: %02x ", din[0]);
	for (i = 1; i < din_len; i++) {
		din[i] = inb(dev->lpc_param - 1 + i);
		debug_trace("%02x ", din[i]);
	}
	debug_trace("\n");

	return 0;
}
#endif

/**
 * Send a MKBP message and receive a reply.
 *
 * @param dev		MKBP device
 * @param din		Data input, containing message to send
 * @param din_len	Length of data input
 * @param dout		Data output buffer, which will contain reply
 * @param replyp	Set to point to start of reply in dout
 * @return number of bytes in response, or -1 on error
 */
static int mkbp_send_message(struct mkbp_dev *dev, uint8_t din[], int din_len,
			     uint8_t dout[], const uint8_t **replyp)
{
	const uint8_t *p, *end;
	int len = 0;

	memset(din, '\0', din_len);

	switch (dev->interface) {
#ifdef CONFIG_MKBP_SPI
	case MKBPIF_SPI:
		if (spi_claim_bus(dev->spi)) {
			debug("%s: Cannot claim SPI bus\n", __func__);
			return -1;
		}
#ifdef CONFIG_NEW_SPI_XFER
		if (spi_xfer(dev->spi, dout, din_len * 8, din, din_len * 8)) {
#else
		if (spi_xfer(dev->spi, din_len * 8, dout, din,
					SPI_XFER_BEGIN | SPI_XFER_END)) {
#endif
			debug("%s: Cannot complete SPI transfer\n", __func__);
			return -1;
		}
		spi_release_bus(dev->spi);
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		din_len += 2; /* Add space for checksum and return code */
		if (mkbp_i2c_message(dev, din, din_len, dout))
			return -1;
		*replyp = &din[1]; /* skip return code */
		return din_len - 2; /* ignore checksum/return code */
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		if (mkbp_lpc_message(dev, din, din_len, dout))
			return -1;
		*replyp = &din[1]; /* skip return code */
		return din_len;
#endif
	case MKBPIF_NONE:
	default:
		return -1;
	}

	/* Scan to start of reply */
	for (p = din, end = p + din_len; p < end; p++) {
		if (*p == MSG_HEADER)
			break;
	}

	if (end - p > 2)
		len = p[1] + (p[2] << 8);
	else
		debug("%s: Message header not found\n", __func__);

	if (!len || len > end - p) {
		debug("%s: Message received is too short, len=%d, bytes=%d\n",
		      __func__, len, end - p);
		len = -1;
	}

	if (len > 0) {
		p += MSG_HEADER_BYTES;
		len -= MSG_PROTO_BYTES;	/* remove header, checksum, trailer */
		*replyp = p;
	}
	return len;
}

/**
 * Send a command to the MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (CMDC_...)
 * @param maxlen	Maximum number of bytes in response
 * @param responsep	Set to point to the response on success
 * @return number of bytes in response, or -1 on error
 */
static int mkbp_send_command(struct mkbp_dev *dev, enum message_cmd_t cmd,
			     int maxlen, const uint8_t **responsep)
{
	int len;

	dev->dout[0] = cmd;
	len = mkbp_send_message(dev, dev->din, sizeof(dev->din), dev->dout,
				responsep);
	if (len < 0)
		return -1;

	len = min(len, maxlen);
	return len;
}

int mkbp_scan_keyboard(struct mkbp_dev *dev, struct mbkp_keyscan *scan)
{
	const uint8_t *p;
	int len;

	len = mkbp_send_command(dev, CMDC_KEY_STATE, sizeof(scan->data), &p);
	if (len > 0)
		memcpy(scan->data, p, len);

	return 0;
}

int mkbp_read_id(struct mkbp_dev *dev, char *id, int maxlen)
{
	const uint8_t *p;
	int len;

	/* Allow room for our \0 terminator */
	len = mkbp_send_command(dev, CMDC_ID, maxlen - 1, &p);
	if (len < 0)
		return -1;
	memcpy(id, p, len);
	id[len] = '\0';

	return 0;
}

int mkbp_interrupt_pending(struct mkbp_dev *dev)
{
	/* no interrupt support : always poll */
	if (!fdt_gpio_isvalid(&dev->ec_int))
		return 1;

	return !gpio_get_value(dev->ec_int.gpio);
}

int mkbp_info(struct mkbp_dev *dev, struct mbkp_info *info)
{
	const uint8_t *p;
	int len;

	len = mkbp_send_command(dev, CMDC_INFO, sizeof(*info), &p);
	if (len >= 0)
		memcpy(info, p, len);
	else
		return -1;

	return 0;
}

/**
 * Decode MBKP details from the device tree and allocate a suitable device.
 *
 * @param blob		Device tree blob
 * @param node		Node to decode from
 * @param devp		Returns a pointer to the new allocated device
 * @return 0 if ok, -1 on error
 */
static int mkbp_decode_fdt(const void *blob, int node, struct mkbp_dev **devp)
{
	enum mkbp_interface_t interface;
	enum fdt_compat_id compat;
	struct mkbp_dev *dev;
	int parent;

	/* See what type of parent we are inside (this is expensive) */
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	dev = &static_dev;

	compat = fdtdec_lookup(blob, parent);
	switch (compat) {
#ifdef CONFIG_MKBP_SPI
	case COMPAT_SAMSUNG_EXYNOS_SPI:
		interface = MKBPIF_SPI;
		dev->max_frequency = fdtdec_get_int(blob, node,
						"spi-max-frequency", 500000);
		dev->cs = fdtdec_get_int(blob, node, "reg", 0);
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case COMPAT_SAMSUNG_S3C2440_I2C:
		interface = MKBPIF_I2C;
		dev->max_frequency = fdtdec_get_int(blob, node,
						    "i2c-max-frequency",
						    100000);
		dev->bus_num = i2c_get_bus_num_fdt(blob, parent);
		dev->addr = fdtdec_get_int(blob, node, "reg", 0);
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case COMPAT_INTEL_LPC: {
		int len, byte, i;
		const u32 *reg;

		interface = MKBPIF_LPC;
		reg = fdt_getprop(blob, node, "reg", &len);
		if (len < sizeof(u32) * 6) {
			debug("%s: LPC reg property is too small\n", __func__);
			return -1;
		}
		byte = 0xff;
		dev->lpc_cmd = fdt32_to_cpu(reg[0]);
		dev->lpc_data = fdt32_to_cpu(reg[2]);
		dev->lpc_param = fdt32_to_cpu(reg[4]);
		dev->lpc_param_len = fdt32_to_cpu(reg[5]);
		byte &= inb(dev->lpc_cmd);
		byte &= inb(dev->lpc_data);
		for (i = 0; i < dev->lpc_param_len; i++)
			byte &= inb(dev->lpc_param + i);
		if (byte == 0xff) {
			debug("%s: MKBP device not found on LPC bus\n",
			      __func__);
			return -1;
		}
		break;
	}
#endif
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}

	fdtdec_decode_gpio(blob, node, "ec-interrupt", &dev->ec_int);
	dev->interface = interface;
	dev->parent_node = parent;
	*devp = dev;

	return 0;
}

struct mkbp_dev *mkbp_init(const void *blob)
{
	char id[MSG_BYTES];
	struct mkbp_dev *dev;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_GOOGLE_MKBP);
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return NULL;
	}
	if (mkbp_decode_fdt(blob, node, &dev)) {
		debug("%s: Failed to decode device.\n", __func__);
		return NULL;
	}

	/* Remember this device for use by the mkbp command */
	last_dev = dev;

	switch (dev->interface) {
#ifdef CONFIG_MKBP_SPI
	case MKBPIF_SPI:
		dev->spi = spi_setup_slave_fdt(blob, dev->parent_node,
					dev->cs, dev->max_frequency, 0);
		if (!dev->spi) {
			debug("%s: Could not setup SPI slave\n", __func__);
			return NULL;
		}
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		i2c_init(dev->max_frequency, dev->addr);
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		break;
#endif
	case MKBPIF_NONE:
	default:
		return NULL;
	}

	/* TODO(sjg@chromium.org): Wait for Bill's x86 GPIO patch */
#if defined(CONFIG_MKBP_SPI) || defined(CONFIG_MKBP_I2C)
	/* we will poll the EC interrupt line */
	fdtdec_setup_gpio(&dev->ec_int);
	if (fdt_gpio_isvalid(&dev->ec_int))
		gpio_direction_input(dev->ec_int.gpio);
#endif
	if (mkbp_read_id(dev, id, sizeof(id))) {
		debug("%s: Could not read KBC ID\n", __func__);
		return NULL;
	}
	debug("Google Matrix Keyboard ready, id '%s'\n", id);

	return dev;
}

#ifdef CONFIG_CMD_MKBP
static int do_mkbp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mkbp_dev *dev = last_dev;
	const char *cmd;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Just use the last allocated device; there should be only one */
	if (!last_dev) {
		printf("No MKBP device available\n");
		return 1;
	}
	cmd = argv[1];
	if (0 == strcmp("id", cmd)) {
		char id[MSG_BYTES];

		if (mkbp_read_id(dev, id, sizeof(id))) {
			debug("%s: Could not read KBC ID\n", __func__);
			return 1;
		}
		printf("%s\n", id);
	} else if (0 == strcmp("info", cmd)) {
		struct mbkp_info info;

		if (mkbp_info(dev, &info)) {
			debug("%s: Could not read KBC info\n", __func__);
			return 1;
		}
		printf("rows     = %u\n", info.rows);
		printf("cols     = %u\n", info.cols);
		printf("switches = %#x\n", info.switches);
	}

	return 0;
}

U_BOOT_CMD(
	mkbp,	5,	1,	do_mkbp,
	"MKBP utility command",
	"id        Read MKBP ID\n"
	"mkbp info      Read MKBP info"
);
#endif
