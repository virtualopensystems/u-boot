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
	int lpc_memmap;			/* Memory mapped area */
	int lpc_memmap_len;		/* Length of memory mapped area */
	uint8_t din[MSG_BYTES];		/* Input data buffer */
	uint8_t dout[MSG_BYTES];	/* Output data buffer */
};

static struct mkbp_dev static_dev, *last_dev;

#ifdef CONFIG_MKBP_I2C
/**
 * Send a command to an I2C MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int i2c_command(struct mkbp_dev *dev, uint8_t cmd, const uint8_t *dout,
		       int dout_len, uint8_t *din, int din_len)
{
	int old_bus = 0;
	int out_bytes = dout_len + 1;  /* cmd8, out8[dout_len] */
	int in_bytes = din_len + 2;  /* response8, in8[din_len], checksum8 */

	old_bus = i2c_get_bus_num();

	/*
	 * Sanity-check I/O sizes given transaction overhead in internal
	 * buffers.
	 */
	if (out_bytes > sizeof(dev->dout)) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/*
	 * Copy command and data into output buffer so we can do a single I2C
	 * burst transaction.
	 */
	dev->dout[0] = cmd;
	if (dout_len > 0)
		memcpy(dev->dout + 1, dout, dout_len);

	/* Set to the proper i2c bus */
	if (i2c_set_bus_num(dev->bus_num)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			dev->bus_num);
		return -1;
	}

	/* Send output data */
	if (i2c_write(dev->addr, 0, 0, dev->dout, out_bytes)) {
		debug("%s: Cannot complete I2C write to 0x%x\n",
			__func__, dev->addr);
		return -1;
	}

	/* Receive input data */
	if (i2c_read(dev->addr, 0, 0, dev->din, in_bytes)) {
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
	 * deal with the full header; the checksum byte on the end
	 * is ignored.
	 * TODO(bhthompson): Migrate to new protocol.
	 */
	if (dev->din[0] != EC_RES_SUCCESS)
		return -(int)(dev->din[0]);

	/* Copy input data, if any */
	if (din_len)
		memcpy(din, dev->din + 1, din_len);

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

/**
 * Send a command to a LPC MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int lpc_command(struct mkbp_dev *dev, uint8_t cmd, const uint8_t *dout,
		       int dout_len, uint8_t *din, int din_len)
{
	int ret, i;

	if (dout_len > dev->lpc_param_len) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	if (din_len > dev->lpc_param_len) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	debug_trace("cmd: %02x, ", command);
	for (i = 0; i < dout_len; i++) {
		debug_trace("%02x ", dout[i]);
		outb(dout[i], dev->lpc_param + i);
	}
	outb(cmd, dev->lpc_cmd);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	ret = inb(dev->lpc_data);
	if (ret) {
		debug("%s: MKBP result code %d\n", __func__, ret);
		return -ret;
	}

	debug_trace("resp: %02x, ", ret);
	for (i = 0; i < din_len; i++) {
		din[i] = inb(dev->lpc_param + i);
		debug_trace("%02x ", din[i]);
	}
	debug_trace("\n");

	return din_len;
}
#endif

#ifdef CONFIG_MKBP_SPI
/**
 * Send a command to a LPC MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int spi_command(struct mkbp_dev *dev, uint8_t cmd, const uint8_t *dout,
		       int dout_len, uint8_t *din, int din_len)
{
	int in_bytes = din_len + MSG_PROTO_BYTES;
	const uint8_t *p, *end;
	int len = 0;
	int rv;

	/*
	 * Sanity-check input size to make sure it plus transaction overhead
	 * fits in the internal device buffer.
	 */
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/* Clear input buffer so we don't get false hits for MSG_HEADER */
	memset(dev->din, '\0', in_bytes);

	if (spi_claim_bus(dev->spi)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}


	/* Send command */
#ifdef CONFIG_NEW_SPI_XFER
	rv = spi_xfer(dev->spi, &cmd, 8, dev->din, 8));
#else
	rv = spi_xfer(dev->spi, 8, &cmd, dev->din,
		      SPI_XFER_BEGIN | SPI_XFER_END);
#endif

	if (rv) {
		debug("%s: Cannot send command via SPI\n", __func__);
		spi_release_bus(dev->spi);
		return -1;
	}

	/* Send output data and receive input data */
#ifdef CONFIG_NEW_SPI_XFER
	rv = spi_xfer(dev->spi, dout, dout_len * 8,
		      dev->din, in_bytes * 8)) {
#else
	/*
	 * TODO(sjg): Old SPI XFER doesn't support sending params before
	 * reading response, so this will probably clock the wrong amount of
	 * data.
	 */
	rv = spi_xfer(dev->spi, max(dout_len, in_bytes) * 8,
		      dev->dout, dev->din,
		      SPI_XFER_BEGIN | SPI_XFER_END);
#endif

	spi_release_bus(dev->spi);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	/* Scan to start of reply */
	for (p = dev->din, end = p + in_bytes; p < end; p++) {
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

		/* Response code is first byte of message */
		if (p[0] != EC_RES_SUCCESS)
			return -(int)(p[0]);

		/* Anything else is the response data */
		len = min(len - 1, din_len);
		if (len)
			memcpy(din, p + 1, len);
	}
	return len;
}
#endif


/**
 * Send a command to the MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int ec_command(struct mkbp_dev *dev, int cmd, const void *dout,
		      int dout_len, void *din, int din_len)
{
	switch (dev->interface) {
#ifdef CONFIG_MKBP_SPI
	case MKBPIF_SPI:
		return spi_command(dev, cmd, (const uint8_t *)dout, dout_len,
				   (uint8_t *)din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		return i2c_command(dev, cmd, (const uint8_t *)dout, dout_len,
				   (uint8_t *)din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		return lpc_command(dev, cmd, (const uint8_t *)dout, dout_len,
				   (uint8_t *)din, din_len);
		break;
#endif
	case MKBPIF_NONE:
	default:
		return -1;
	}
}

int mkbp_scan_keyboard(struct mkbp_dev *dev, struct mbkp_keyscan *scan)
{
	return ec_command(dev, EC_CMD_MKBP_STATE, NULL, 0, scan->data,
			  sizeof(scan->data));
}

int mkbp_read_id(struct mkbp_dev *dev, char *id, int maxlen)
{
	struct ec_response_get_version r;

	if (ec_command(dev, EC_CMD_GET_VERSION, NULL, 0, &r, sizeof(r)) < 0)
		return -1;

	if (maxlen > sizeof(r.version_string_ro))
		maxlen = sizeof(r.version_string_ro);

	switch (r.current_image) {
	case EC_IMAGE_RO:
		memcpy(id, r.version_string_ro, maxlen);
		break;
	case EC_IMAGE_RW_A:
		memcpy(id, r.version_string_rw_a, maxlen);
		break;
	case EC_IMAGE_RW_B:
		memcpy(id, r.version_string_rw_b, maxlen);
		break;
	default:
		return -1;
	}

	id[maxlen - 1] = '\0';
	return 0;
}

int mkbp_read_current_image(struct mkbp_dev *dev, enum ec_current_image *image)
{
	struct ec_response_get_version r;

	if (ec_command(dev, EC_CMD_GET_VERSION, NULL, 0, &r, sizeof(r)) < 0)
		return -1;

	*image = r.current_image;
	return 0;
}

int mkbp_read_hash(struct mkbp_dev *dev, struct ec_response_vboot_hash *hash)
{
	struct ec_params_vboot_hash p;

	p.cmd = EC_VBOOT_HASH_GET;

	if (ec_command(dev, EC_CMD_VBOOT_HASH, &p, sizeof(p),
		       hash, sizeof(*hash)) < 0)
		return -1;

	if (hash->status != EC_VBOOT_HASH_STATUS_DONE) {
		debug("%s: Hash status not done: %d\n", __func__,
		      hash->status);
		return -1;
	}

	return 0;
}

int mkbp_reboot(struct mkbp_dev *dev, enum ec_reboot_cmd cmd, uint8_t flags)
{
	struct ec_params_reboot_ec p;

	p.cmd = cmd;
	p.flags = flags;

	if (ec_command(dev, EC_CMD_REBOOT_EC, &p, sizeof(p), NULL, 0) < 0)
		return -1;

	if (!(flags & EC_REBOOT_FLAG_ON_AP_SHUTDOWN)) {
		/*
		 * EC reboot will take place immediately so delay to allow it
		 * to complete.  Note that some reboot types (EC_REBOOT_COLD)
		 * will reboot the AP as well, in which case we won't actually
		 * get to this point.
		 */
		/*
		 * TODO(rspangler@chromium.org): Would be nice if we had a
		 * better way to determine when the reboot is complete.  Could
		 * we poll a memory-mapped LPC value?
		 */
		udelay(50000);
	}

	return 0;
}

int mkbp_interrupt_pending(struct mkbp_dev *dev)
{
	/* no interrupt support : always poll */
	if (!fdt_gpio_isvalid(&dev->ec_int))
		return 1;

	return !gpio_get_value(dev->ec_int.gpio);
}

int mkbp_info(struct mkbp_dev *dev, struct ec_response_mkbp_info *info)
{
	return ec_command(dev, EC_CMD_MKBP_INFO, NULL, 0, info, sizeof(*info));
}

int mkbp_get_host_events(struct mkbp_dev *dev, uint32_t *events_ptr)
{
	struct ec_response_host_event_mask resp;

	/*
	 * Use the B copy of the event flags, because the main copy is already
	 * used by ACPI/SMI.
	 */
	if (ec_command(dev, EC_CMD_HOST_EVENT_GET_B, NULL, 0,
		       &resp, sizeof(resp)) < 0)
		return -1;

	if (resp.mask & EC_HOST_EVENT_MASK(EC_HOST_EVENT_INVALID))
		return -1;

	*events_ptr = resp.mask;
	return 0;
}

int mkbp_clear_host_events(struct mkbp_dev *dev, uint32_t events)
{
	struct ec_params_host_event_mask params;

	params.mask = events;

	/*
	 * Use the B copy of the event flags, so it affects the data returned
	 * by mkbp_get_host_events().
	 */
	return ec_command(dev, EC_CMD_HOST_EVENT_CLEAR_B,
			  &params, sizeof(params), NULL, 0);
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
		if (len < sizeof(u32) * 8) {
			debug("%s: LPC reg property is too small\n", __func__);
			return -1;
		}
		byte = 0xff;
		dev->lpc_cmd = fdt32_to_cpu(reg[0]);
		dev->lpc_data = fdt32_to_cpu(reg[2]);
		dev->lpc_param = fdt32_to_cpu(reg[4]);
		dev->lpc_param_len = fdt32_to_cpu(reg[5]);
		dev->lpc_memmap = fdt32_to_cpu(reg[6]);
		dev->lpc_memmap_len = fdt32_to_cpu(reg[7]);
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
	debug("Google Chrome EC MKBP driver ready, id '%s'\n", id);

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
		struct ec_response_mkbp_info info;

		if (mkbp_info(dev, &info)) {
			debug("%s: Could not read KBC info\n", __func__);
			return 1;
		}
		printf("rows     = %u\n", info.rows);
		printf("cols     = %u\n", info.cols);
		printf("switches = %#x\n", info.switches);
	} else if (0 == strcmp("curimage", cmd)) {
		enum ec_current_image image;

		if (mkbp_read_current_image(dev, &image)) {
			debug("%s: Could not read KBC image\n", __func__);
			return 1;
		}
		printf("%d\n", image);
	} else if (0 == strcmp("hash", cmd)) {
		struct ec_response_vboot_hash hash;
		int i;

		if (mkbp_read_hash(dev, &hash)) {
			debug("%s: Could not read KBC hash\n", __func__);
			return 1;
		}

		if (hash.hash_type == EC_VBOOT_HASH_TYPE_SHA256)
			printf("type:    SHA-256\n");
		else
			printf("type:    %d\n", hash.hash_type);

		printf("offset:  0x%08x\n", hash.offset);
		printf("size:    0x%08x\n", hash.size);

		printf("digest:  ");
		for (i = 0; i < hash.digest_size; i++)
			printf("%02x", hash.hash_digest[i]);
		printf("\n");
	} else if (0 == strcmp("reboot", cmd)) {
		enum ec_reboot_cmd cmd = EC_REBOOT_COLD;

		if (argc >= 3 && !strcmp(argv[2], "rw"))
			cmd = EC_REBOOT_JUMP_RW_A;

		if (mkbp_reboot(dev, cmd, 0)) {
			debug("%s: Could not reboot KBC\n", __func__);
			return 1;
		}
	} else if (0 == strcmp("events", cmd)) {
		uint32_t events;

		if (mkbp_get_host_events(dev, &events)) {
			debug("%s: Could not read host events\n", __func__);
			return 1;
		}
		printf("0x%08x\n", events);
	} else if (0 == strcmp("clrevents", cmd)) {
		uint32_t events = 0x7fffffff;

		if (argc >= 3)
			events = simple_strtol(argv[2], NULL, 0);

		if (mkbp_clear_host_events(dev, events)) {
			debug("%s: Could not clear host events\n", __func__);
			return 1;
		}
	} else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	mkbp,	5,	1,	do_mkbp,
	"MKBP utility command",
	"id                       Read MKBP ID\n"
	"mkbp info                Read MKBP info\n"
	"mkbp curimage            Read MKBP current image\n"
	"mkbp hash                Read MKBP hash\n"
	"mkbp reboot [rw | cold]  Reboot MKBP\n"
	"mkbp events              Read MKBP host events\n"
	"mkbp clrevents [mask]    Clear MKBP host events"
);
#endif
