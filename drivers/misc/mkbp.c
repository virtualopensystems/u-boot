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
#include <mkbp.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <asm/arch-exynos/spi.h>


/* Which interface is the device on? */
enum mkbp_interface_t {
	MKBPIF_NONE,
	MKBPIF_SPI,
};

/* Our configuration information */
struct mkbp_dev {
	enum mkbp_interface_t interface;
	struct spi_slave *spi;		/* Our SPI slave, if using SPI */
	int parent_node;		/* Our parent node (interface) */
	unsigned int cs;		/* Our chip select */
	unsigned int max_frequency;	/* Maximum SPI frequency */
	uint8_t din[MSG_BYTES];		/* Input data buffer */
	uint8_t dout[MSG_BYTES];	/* Output data buffer */
};

static struct mkbp_dev *last_dev;

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
	spi_release_bus(dev->spi);

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
	if (len >= 0) {
		memcpy(id, p, len);
		id[len] = '\0';
	}

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

	compat = fdtdec_lookup(blob, parent);
	switch (compat) {
	case COMPAT_SAMSUNG_EXYNOS_SPI:
		interface = MKBPIF_SPI;
		break;
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}

	dev = (struct mkbp_dev *)calloc(1, sizeof(*dev));
	if (!dev) {
		debug("%s: Cannot allocate memory\n", __func__);
		return -1;
	}
	dev->interface = interface;
	dev->cs = fdtdec_get_int(blob, node, "reg", 0);
	dev->max_frequency = fdtdec_get_int(blob, node, "spi-max-frequency",
					    500000);

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
	if (mkbp_decode_fdt(blob, node, &dev))
		return NULL;

	/* Remember this device for use by the mkbp command */
	last_dev = dev;

	switch (dev->interface) {
	case MKBPIF_NONE:
		return NULL;
	case MKBPIF_SPI:
		dev->spi = spi_setup_slave_fdt(blob, dev->parent_node,
					dev->cs, dev->max_frequency, 0);
		if (!dev->spi) {
			debug("%s: Could not setup SPI slave\n", __func__);
			return NULL;
		}
		if (mkbp_read_id(dev, id, sizeof(id))) {
			debug("%s: Could not read KBC ID\n", __func__);
			return NULL;
		}
		break;
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
	}

	return 0;
}

U_BOOT_CMD(
	mkbp,	5,	1,	do_mkbp,
	"MKBP utility command",
	"id        Read MKBP ID"
);
#endif
