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

static struct mkbp_dev static_dev, *last_dev;

/**
 * Send a command to the MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int ec_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		      const void *dout, int dout_len,
		      void *din, int din_len)
{
	if (cmd_version != 0 && !dev->cmd_version_is_supported) {
		debug("%s: Command version >0 unsupported\n", __func__);
		return -1;
	}

	switch (dev->interface) {
#ifdef CONFIG_MKBP_SPI
	case MKBPIF_SPI:
		return mkbp_spi_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
					(uint8_t *)din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		return mkbp_i2c_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
					(uint8_t *)din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		return mkbp_lpc_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
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
	if (ec_command(dev, EC_CMD_MKBP_STATE, 0, NULL, 0, scan->data,
		       sizeof(scan->data)) < sizeof(scan->data))
		return -1;

	return 0;
}

int mkbp_read_id(struct mkbp_dev *dev, char *id, int maxlen)
{
	struct ec_response_get_version r;

	if (ec_command(dev, EC_CMD_GET_VERSION, 0, NULL, 0, &r,
		       sizeof(r)) < sizeof(r))
		return -1;

	if (maxlen > sizeof(r.version_string_ro))
		maxlen = sizeof(r.version_string_ro);

	switch (r.current_image) {
	case EC_IMAGE_RO:
		memcpy(id, r.version_string_ro, maxlen);
		break;
	case EC_IMAGE_RW:
		memcpy(id, r.version_string_rw, maxlen);
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

	if (ec_command(dev, EC_CMD_GET_VERSION, 0, NULL, 0, &r,
		       sizeof(r)) < sizeof(r))
		return -1;

	*image = r.current_image;
	return 0;
}

int mkbp_read_hash(struct mkbp_dev *dev, struct ec_response_vboot_hash *hash)
{
	struct ec_params_vboot_hash p;

	p.cmd = EC_VBOOT_HASH_GET;

	if (ec_command(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
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

	if (ec_command(dev, EC_CMD_REBOOT_EC, 0, &p, sizeof(p), NULL, 0) < 0)
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
	if (ec_command(dev, EC_CMD_MKBP_INFO, 0,
		       NULL, 0, info, sizeof(*info)) < sizeof(*info))
		return -1;

	return 0;
}

int mkbp_get_host_events(struct mkbp_dev *dev, uint32_t *events_ptr)
{
	struct ec_response_host_event_mask resp;

	/*
	 * Use the B copy of the event flags, because the main copy is already
	 * used by ACPI/SMI.
	 */
	if (ec_command(dev, EC_CMD_HOST_EVENT_GET_B, 0, NULL, 0,
		       &resp, sizeof(resp)) < sizeof(resp))
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
	if (ec_command(dev, EC_CMD_HOST_EVENT_CLEAR_B, 0,
		       &params, sizeof(params), NULL, 0) < 0)
		return -1;

	return 0;
}

int mkbp_flash_protect(struct mkbp_dev *dev,
		       uint32_t set_mask, uint32_t set_flags,
		       struct ec_response_flash_protect *resp)
{
	struct ec_params_flash_protect params;

	params.mask = set_mask;
	params.flags = set_flags;

	if (ec_command(dev, EC_CMD_FLASH_PROTECT, EC_VER_FLASH_PROTECT,
		       &params, sizeof(params),
		       resp, sizeof(*resp)) < sizeof(*resp))
		return -1;

	return 0;
}

int mkbp_test(struct mkbp_dev *dev)
{
	struct ec_params_hello req;
	struct ec_response_hello resp;

	req.in_data = 0x12345678;
	if (ec_command(dev, EC_CMD_HELLO, 0, (uint8_t **)&req, sizeof(req),
		       &resp, sizeof(resp)) < sizeof(resp)) {
		printf("ec_command() returned error\n");
		return -1;
	}
	if (resp.out_data != req.in_data + 0x01020304) {
		printf("Received invalid handshake %x\n", resp.out_data);
		return -1;
	}

	return 0;
}

static int mkbp_flash_rw_offset(struct mkbp_dev *dev,
				uint32_t *offset, uint32_t *size)
{
	struct ec_params_flash_region_info p;
	struct ec_response_flash_region_info r;
	int ret;

	p.region = EC_FLASH_REGION_RW;
	ret = ec_command(dev, EC_CMD_FLASH_REGION_INFO,
			 EC_VER_FLASH_REGION_INFO,
			 &p, sizeof(p), &r, sizeof(r));
	if (ret != sizeof(r))
		return -1;

	if (offset)
		*offset = r.offset;
	if (size)
		*size = r.size;

	return 0;
}

static int mkbp_flash_erase(struct mkbp_dev *dev,
			    uint32_t offset, uint32_t size)
{
	struct ec_params_flash_erase p;

	p.offset = offset;
	p.size = size;
	return ec_command(dev, EC_CMD_FLASH_ERASE, 0,
			  &p, sizeof(p), NULL, 0) >= 0 ? 0 : -1;
}

static int mkbp_flash_write(struct mkbp_dev *dev, const uint8_t  *data,
			    uint32_t offset, uint32_t size)
{
	struct ec_params_flash_write p;

	p.offset = offset;
	p.size = size;
	memcpy(p.data, data, p.size);

	return ec_command(dev, EC_CMD_FLASH_WRITE, 0,
			  &p, sizeof(p), NULL, 0) >= 0 ? 0 : -1;
}

int mkbp_flash_update_rw(struct mkbp_dev *dev,
			 const uint8_t  *image, int image_size)
{
	uint32_t rw_offset, rw_size;
	uint32_t end, off;
	uint32_t burst = sizeof(dev->dout);
	int ret;

	if (mkbp_flash_rw_offset(dev, &rw_offset, &rw_size))
		return -1;
	if (image_size > rw_size)
		return -1;

	rw_size = min(rw_size, image_size);
	ret = mkbp_flash_erase(dev, rw_offset, rw_size);
	if (ret)
		return ret;

	end = rw_offset + rw_size;
	for (off = rw_offset; off < end; off += burst, image += burst) {
		ret = mkbp_flash_write(dev, image, off, min(end - off, burst));
		if (ret)
			return ret;
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
	dev->node = node;
	dev->parent_node = parent;

	compat = fdtdec_lookup(blob, parent);
	switch (compat) {
#ifdef CONFIG_MKBP_SPI
	case COMPAT_SAMSUNG_EXYNOS_SPI:
		dev->interface = MKBPIF_SPI;
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case COMPAT_SAMSUNG_S3C2440_I2C:
		dev->interface = MKBPIF_I2C;
		if (mkbp_i2c_decode_fdt(dev, blob))
			return -1;
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case COMPAT_INTEL_LPC:
		dev->interface = MKBPIF_LPC;
		break;
#endif
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}

	fdtdec_decode_gpio(blob, node, "ec-interrupt", &dev->ec_int);
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
		if (mkbp_spi_init(dev, blob))
			return NULL;
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		if (mkbp_i2c_init(dev, blob))
			return NULL;
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		if (mkbp_lpc_init(dev, blob))
			return NULL;
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
			cmd = EC_REBOOT_JUMP_RW;

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
	} else if (0 == strcmp("test", cmd)) {
		int result = mkbp_test(dev);

		if (result)
			printf("Test failed with error %d\n", result);
		else
			puts("Test passed\n");
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
	"mkbp clrevents [mask]    Clear MKBP host events\n"
	"mkbp test                run tests on mkbp"
);
#endif
