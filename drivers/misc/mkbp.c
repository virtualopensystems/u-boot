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

void mkbp_dump_data(const char *name, int cmd, const uint8_t *data, int len)
{
#ifdef DEBUG
	int i;

	printf("%s: ", name);
	if (cmd != -1)
		printf("cmd=%#x: ", cmd);
	for (i = 0; i < len; i++)
		printf("%02x ", data[i]);
	printf("\n");
#endif
}

/*
 * Calculate a simple 8-bit checksum of a data block
 *
 * @param data	Data block to checksum
 * @param size	Size of data block in bytes
 * @return checksum value (0 to 255)
 */
int mkbp_calc_checksum(const uint8_t *data, int size)
{
	int csum, i;

	for (i = csum = 0; i < size; i++)
		csum += data[i];
	return csum & 0xff;
}

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
 * @param dinp          Response data (may be NULL If din_len=0).
 *			The value of *dinp is a place for ec_command() to put
 *			the data (it will be copied there). If NULL on entry,
 *			then it will be updated to point to the data (no copy)
 *			and will always be double word aligned (64-bits)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int ec_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		      const void *dout, int dout_len,
		      uint8_t **dinp, int din_len)
{
	uint8_t *din;
	int len;

	if (cmd_version != 0 && !dev->cmd_version_is_supported) {
		debug("%s: Command version >0 unsupported\n", __func__);
		return -1;
	}

	switch (dev->interface) {
#ifdef CONFIG_MKBP_SPI
	case MKBPIF_SPI:
		len = mkbp_spi_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
					&din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_I2C
	case MKBPIF_I2C:
		len = mkbp_i2c_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
					&din, din_len);
		break;
#endif
#ifdef CONFIG_MKBP_LPC
	case MKBPIF_LPC:
		len = mkbp_lpc_command(dev, cmd, cmd_version,
					(const uint8_t *)dout, dout_len,
					&din, din_len);
		break;
#endif
	case MKBPIF_NONE:
	default:
		return -1;
	}

	/*
	 * If we were asked to put it somewhere, do so, otherwise just
	 * return a pointer to the data in dinp.
	 */
	debug("%s: len=%d, din=%p, dinp=%p, *dinp=%p\n", __func__,
		len, din, dinp, dinp ? *dinp : NULL);
	if (dinp) {
		/* If we have any data to return, it must be 64bit-aligned */
		assert(len <= 0 || !((uintptr_t)din & 7));
		if (len > 0) {
			if (*dinp)
				memmove(*dinp, din, len);
			else
				*dinp = din;
		}
	}

	return len;
}

int mkbp_scan_keyboard(struct mkbp_dev *dev, struct mbkp_keyscan *scan)
{
	if (ec_command(dev, EC_CMD_MKBP_STATE, 0, NULL, 0, (uint8_t **)&scan,
		       sizeof(scan->data)) < sizeof(scan->data))
		return -1;

	return 0;
}

int mkbp_read_id(struct mkbp_dev *dev, char *id, int maxlen)
{
	struct ec_response_get_version *r = NULL;

	if (ec_command(dev, EC_CMD_GET_VERSION, 0, NULL, 0, (uint8_t **)&r,
		       sizeof(*r)) < sizeof(*r))
		return -1;

	if (maxlen > sizeof(r->version_string_ro))
		maxlen = sizeof(r->version_string_ro);

	switch (r->current_image) {
	case EC_IMAGE_RO:
		memcpy(id, r->version_string_ro, maxlen);
		break;
	case EC_IMAGE_RW:
		memcpy(id, r->version_string_rw, maxlen);
		break;
	default:
		return -1;
	}

	id[maxlen - 1] = '\0';
	return 0;
}

int mkbp_read_current_image(struct mkbp_dev *dev, enum ec_current_image *image)
{
	struct ec_response_get_version *r = NULL;

	if (ec_command(dev, EC_CMD_GET_VERSION, 0, NULL, 0, (uint8_t **)&r,
		       sizeof(*r)) < sizeof(*r))
		return -1;

	*image = r->current_image;
	return 0;
}

int mkbp_read_hash(struct mkbp_dev *dev, struct ec_response_vboot_hash *hash)
{
	struct ec_params_vboot_hash p;

	p.cmd = EC_VBOOT_HASH_GET;

	if (ec_command(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
		       (uint8_t **)&hash, sizeof(*hash)) < 0)
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
			NULL, 0, (uint8_t **)&info, sizeof(*info))
				< sizeof(*info))
		return -1;

	return 0;
}

int mkbp_get_host_events(struct mkbp_dev *dev, uint32_t *events_ptr)
{
	struct ec_response_host_event_mask *resp = NULL;

	/*
	 * Use the B copy of the event flags, because the main copy is already
	 * used by ACPI/SMI.
	 */
	if (ec_command(dev, EC_CMD_HOST_EVENT_GET_B, 0, NULL, 0,
		       (uint8_t **)&resp, sizeof(*resp)) < sizeof(*resp))
		return -1;

	if (resp->mask & EC_HOST_EVENT_MASK(EC_HOST_EVENT_INVALID))
		return -1;

	*events_ptr = resp->mask;
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
		       (uint8_t **)&resp, sizeof(*resp)) < sizeof(*resp))
		return -1;

	return 0;
}

static int mkbp_check_version(struct mkbp_dev *dev)
{
	struct ec_params_hello req;
	struct ec_response_hello *resp = NULL;

#ifdef CONFIG_MKBP_LPC
	/* LPC has its own way of doing this */
	if (dev->interface == MKBPIF_LPC)
		return mkbp_lpc_check_version(dev);
#endif

	/*
	 * TODO(sjg@chromium.org).
	 * There is a strange oddity here with the EC. We could just ignore
	 * the response, i.e. pass the last two parameters as NULL and 0.
	 * In this case we won't read back very many bytes from the EC.
	 * On the I2C bus the EC gets upset about this and will try to send
	 * the bytes anyway. This means that we will have to wait for that
	 * to complete before continuing with a new EC command.
	 *
	 * This problem is probably unique to the I2C bus.
	 *
	 * So for now, just read all the data anyway.
	 */
	dev->cmd_version_is_supported = 1;
	if (ec_command(dev, EC_CMD_HELLO, 0, &req, sizeof(req),
		       (uint8_t **)&resp, sizeof(*resp)) > 0) {
		/* It appears to understand new version commands */
		dev->cmd_version_is_supported = 1;
	} else {
		dev->cmd_version_is_supported = 0;
		if (ec_command(dev, EC_CMD_HELLO, 0, &req,
			      sizeof(req), (uint8_t **)&resp,
			      sizeof(*resp)) < 0) {
			debug("%s: Failed both old and new command style\n",
				__func__);
			return -1;
		}
	}

	return 0;
}

int mkbp_test(struct mkbp_dev *dev)
{
	struct ec_params_hello req;
	struct ec_response_hello *resp = NULL;

	req.in_data = 0x12345678;
	if (ec_command(dev, EC_CMD_HELLO, 0, &req, sizeof(req),
		       (uint8_t **)&resp, sizeof(*resp)) < sizeof(*resp)) {
		printf("ec_command() returned error\n");
		return -1;
	}
	if (resp->out_data != req.in_data + 0x01020304) {
		printf("Received invalid handshake %x\n", resp->out_data);
		return -1;
	}

	return 0;
}

/**
 * Obtain position and size of a flash region
 *
 * @param dev		MKBP device
 * @param region	Flash region to query
 * @param offset	Returns offset of flash region in EC flash
 * @param size		Returns size of flash region
 * @return 0 if ok, -1 on error
 */
static int mkbp_flash_offset(struct mkbp_dev *dev, enum ec_flash_region region,
			     uint32_t *offset, uint32_t *size)
{
	struct ec_params_flash_region_info p;
	struct ec_response_flash_region_info *r = NULL;
	int ret;

	p.region = region;
	ret = ec_command(dev, EC_CMD_FLASH_REGION_INFO,
			 EC_VER_FLASH_REGION_INFO,
			 &p, sizeof(p), (uint8_t **)&r, sizeof(*r));
	if (ret != sizeof(*r))
		return -1;

	if (offset)
		*offset = r->offset;
	if (size)
		*size = r->size;

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

/**
 * Write a single block to the flash
 *
 * Write a block of data to the EC flash. The size must not exceed the flash
 * write block size which you can obtain from mkbp_flash_write_burst_size().
 *
 * The offset starts at 0. You can obtain the region information from
 * mkbp_flash_offset() to find out where to write for a particular region.
 *
 * Attempting to write to the region where the EC is currently running from
 * will result in an error.
 *
 * @param dev		MKBP device
 * @param data		Pointer to data buffer to write
 * @param offset	Offset within flash to write to.
 * @param size		Number of bytes to write
 * @return 0 if ok, -1 on error
 */
static int mkbp_flash_write_block(struct mkbp_dev *dev, const uint8_t *data,
				  uint32_t offset, uint32_t size)
{
	struct ec_params_flash_write p;

	p.offset = offset;
	p.size = size;
	assert(data && p.size <= sizeof(p.data));
	memcpy(p.data, data, p.size);

	return ec_command(dev, EC_CMD_FLASH_WRITE, 0,
			  &p, sizeof(p), NULL, 0) >= 0 ? 0 : -1;
}

/**
 * Return optimal flash write burst size
 */
static int mkbp_flash_write_burst_size(struct mkbp_dev *dev)
{
	struct ec_params_flash_write p;
	return sizeof(p.data);
}

/**
 * Write data to the flash
 *
 * Write an arbitrary amount of data to the EC flash, by repeatedly writing
 * small blocks.
 *
 * The offset starts at 0. You can obtain the region information from
 * mkbp_flash_offset() to find out where to write for a particular region.
 *
 * Attempting to write to the region where the EC is currently running from
 * will result in an error.
 *
 * @param dev		MKBP device
 * @param data		Pointer to data buffer to write
 * @param offset	Offset within flash to write to.
 * @param size		Number of bytes to write
 * @return 0 if ok, -1 on error
 */
static int mkbp_flash_write(struct mkbp_dev *dev, const uint8_t *data,
			    uint32_t offset, uint32_t size)
{
	uint32_t burst = mkbp_flash_write_burst_size(dev);
	uint32_t end, off;
	int ret;

	/*
	 * TODO: round up to the nearest multiple of write size.  Can get away
	 * without that on link right now because its write size is 4 bytes.
	 */
	end = offset + size;
	for (off = offset; off < end; off += burst, data += burst) {
		ret = mkbp_flash_write_block(dev, data, off,
					     min(end - off, burst));
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * Read a single block from the flash
 *
 * Read a block of data from the EC flash. The size must not exceed the flash
 * write block size which you can obtain from mkbp_flash_write_burst_size().
 *
 * The offset starts at 0. You can obtain the region information from
 * mkbp_flash_offset() to find out where to read for a particular region.
 *
 * @param dev		MKBP device
 * @param data		Pointer to data buffer to read into
 * @param offset	Offset within flash to read from
 * @param size		Number of bytes to read
 * @return 0 if ok, -1 on error
 */
static int mkbp_flash_read_block(struct mkbp_dev *dev, uint8_t *data,
				 uint32_t offset, uint32_t size)
{
	struct ec_params_flash_read p;

	p.offset = offset;
	p.size = size;

	return ec_command(dev, EC_CMD_FLASH_READ, 0,
			  &p, sizeof(p), &data, size) >= 0 ? 0 : -1;
}

/**
 * Read data from the flash
 *
 * Read an arbitrary amount of data from the EC flash, by repeatedly reading
 * small blocks.
 *
 * The offset starts at 0. You can obtain the region information from
 * mkbp_flash_offset() to find out where to read for a particular region.
 *
 * @param dev		MKBP device
 * @param data		Pointer to data buffer to read into
 * @param offset	Offset within flash to read from
 * @param size		Number of bytes to read
 * @return 0 if ok, -1 on error
 */
int mkbp_flash_read(struct mkbp_dev *dev, uint8_t *data, uint32_t offset,
		    uint32_t size)
{
	uint32_t burst = mkbp_flash_write_burst_size(dev);
	uint32_t end, off;
	int ret;

	end = offset + size;
	for (off = offset; off < end; off += burst, data += burst) {
		ret = mkbp_flash_read_block(dev, data, off,
					    min(end - off, burst));
		if (ret)
			return ret;
	}

	return 0;
}

int mkbp_flash_update_rw(struct mkbp_dev *dev,
			 const uint8_t *image, int image_size)
{
	uint32_t rw_offset, rw_size;
	int ret;

	if (mkbp_flash_offset(dev, EC_FLASH_REGION_RW, &rw_offset, &rw_size))
		return -1;
	if (image_size > rw_size)
		return -1;

	/*
	 * Erase the entire RW section, so that the EC doesn't see any garbage
	 * past the new image if it's smaller than the current image.
	 *
	 * TODO: could optimize this to erase just the current image, since
	 * presumably everything past that is 0xff's.  But would still need to
	 * round up to the nearest multiple of erase size.
	 */
	ret = mkbp_flash_erase(dev, rw_offset, rw_size);
	if (ret)
		return ret;

	/* Write the image */
	ret = mkbp_flash_write(dev, image, rw_offset, image_size);
	if (ret)
		return ret;

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
		if (mkbp_spi_decode_fdt(dev, blob))
			return -1;
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
		if (mkbp_spi_init(dev, blob)) {
			debug("%s: Could not setup SPI interface\n", __func__);
			return NULL;
		}
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

	/* we will poll the EC interrupt line */
	fdtdec_setup_gpio(&dev->ec_int);
	if (fdt_gpio_isvalid(&dev->ec_int))
		gpio_direction_input(dev->ec_int.gpio);

	if (mkbp_check_version(dev)) {
		debug("%s: Could not detect MKBP version\n", __func__);
		return NULL;
	}

	if (mkbp_read_id(dev, id, sizeof(id))) {
		debug("%s: Could not read KBC ID\n", __func__);
		return NULL;
	}
	debug("Google Chrome EC MKBP driver ready, id '%s'\n", id);

	return dev;
}

#ifdef CONFIG_CMD_MKBP
/**
 * Decode a flash region parameter
 *
 * @param argc	Number of params remaining
 * @param argv	List of remaining parameters
 * @return flash region (EC_FLASH_REGION_...) or -1 on error
 */
static int decode_region(int argc, char * const argv[])
{
	if (argc > 0) {
		if (0 == strcmp(*argv, "rw"))
			return EC_FLASH_REGION_RW;
		else if (0 == strcmp(*argv, "ro"))
			return EC_FLASH_REGION_RO;

		debug("%s: Invalid region '%s'\n", __func__, *argv);
	} else {
		debug("%s: Missing region parameter\n", __func__);
	}

	return -1;
}

static int do_mkbp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mkbp_dev *dev = last_dev;
	const char *cmd;
	int ret = 0;

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
	} else if (0 == strcmp("erase", cmd)) {
		int region = decode_region(argc - 2, argv + 2);
		uint32_t offset, size;

		if (region == -1)
			return CMD_RET_USAGE;
		if (mkbp_flash_offset(dev, region, &offset, &size)) {
			debug("%s: Could not read region info\n", __func__);
			ret = -1;
		} else if (mkbp_flash_erase(dev, offset, size)) {
			debug("%s: Could not erase region\n", __func__);
			ret = -1;
		}
	} else if (0 == strcmp("regioninfo", cmd)) {
		int region = decode_region(argc - 2, argv + 2);
		uint32_t offset, size;

		if (region == -1)
			return CMD_RET_USAGE;
		ret = mkbp_flash_offset(dev, region, &offset, &size);
		if (ret) {
			debug("%s: Could not read region info\n", __func__);
		} else {
			printf("Region: %s\n", region == EC_FLASH_REGION_RO ?
					"RO" : "RW");
			printf("Offset: %x\n", offset);
			printf("Size:   %x\n", size);
		}
	} else if (0 == strcmp("test", cmd)) {
		int result = mkbp_test(dev);

		if (result)
			printf("Test failed with error %d\n", result);
		else
			puts("Test passed\n");
	} else {
		return CMD_RET_USAGE;
	}

	if (ret < 0) {
		printf("Error: MKBP command failed (error %d)\n", ret);
		ret = 1;
	}

	return ret;
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
	"mkbp regioninfo <ro|rw>  Read image info\n"
	"mkbp erase <ro|rw>       Erase EC image\n"
	"mkbp test                run tests on mkbp"
);
#endif
