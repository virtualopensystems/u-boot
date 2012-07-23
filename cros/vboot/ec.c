/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <asm/global_data.h>
#include <cros/common.h>
#include <cros/firmware_storage.h>
#include <cros/vboot_flag.h>
#include <malloc.h>
#include <mkbp.h>
#include <vboot_api.h>

DECLARE_GLOBAL_DATA_PTR;

int VbExTrustEC(void)
{
	struct vboot_flag_details gpio_ec_in_rw;
	int okay;

	/* If we don't have a valid GPIO to read, we can't trust it. */
	if (vboot_flag_fetch(VBOOT_FLAG_EC_IN_RW, &gpio_ec_in_rw)) {
		VBDEBUG("can't find GPIO to read, returning 0\n");
		return 0;
	}

	/* We only trust it if it's NOT in its RW firmware. */
	okay = !gpio_ec_in_rw.value;

	VBDEBUG("port=%d value=%d, returning %d\n",
		gpio_ec_in_rw.port, gpio_ec_in_rw.value, okay);

	return okay;
}

#ifdef CONFIG_MKBP

VbError_t VbExEcRunningRW(int *in_rw)
{
	struct mkbp_dev *mdev = board_get_mkbp_dev();
	enum ec_current_image image;

	if (mkbp_read_current_image(mdev, &image) < 0)
		return VBERROR_UNKNOWN;

	switch (image) {
	case EC_IMAGE_RO:
		*in_rw = 0;
		break;
	case EC_IMAGE_RW:
		*in_rw = 1;
		break;
	default:
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExEcJumpToRW(void)
{
	struct mkbp_dev *mdev = board_get_mkbp_dev();

	if (mkbp_reboot(mdev, EC_REBOOT_JUMP_RW, 0) < 0)
		return VBERROR_UNKNOWN;

	return VBERROR_SUCCESS;
}

VbError_t VbExEcStayInRO(void)
{
	struct mkbp_dev *mdev = board_get_mkbp_dev();

	if (mkbp_reboot(mdev, EC_REBOOT_DISABLE_JUMP, 0) < 0)
		return VBERROR_UNKNOWN;

	return VBERROR_SUCCESS;
}

VbError_t VbExEcHashRW(const uint8_t **hash, int *hash_size)
{
	struct mkbp_dev *mdev = board_get_mkbp_dev();
	static struct ec_response_vboot_hash resp;

	if (mkbp_read_hash(mdev, &resp) < 0)
		return VBERROR_UNKNOWN;

	/*
	 * TODO (rspangler@chromium.org): the code below isn't very tolerant
	 * of errors.
	 *
	 * If the EC is busy calculating a hash, we should wait and retry
	 * reading the hash status.
	 *
	 * If the hash is unavailable, the wrong type, or covers the wrong
	 * offset/size (which we need to get from the FDT, since it's
	 * board-specific), we should request a new hash and wait for it to
	 * finish.
	 */
	if (resp.status != EC_VBOOT_HASH_STATUS_DONE)
		return VBERROR_UNKNOWN;
	if (resp.hash_type != EC_VBOOT_HASH_TYPE_SHA256)
		return VBERROR_UNKNOWN;

	*hash = resp.hash_digest;
	*hash_size = resp.digest_size;

	return VBERROR_SUCCESS;
}

VbError_t VbExEcUpdateRW(const uint8_t  *image, int image_size)
{
	/* TODO: implement me!  crosbug.com/p/11149 */
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcProtectRW(void)
{
	struct mkbp_dev *mdev = board_get_mkbp_dev();
	struct ec_response_flash_protect resp;

	/* Protect all flash code */
	if (mkbp_flash_protect(mdev, EC_FLASH_PROTECT_RW_NOW,
			       EC_FLASH_PROTECT_RW_NOW, &resp) < 0)
		return VBERROR_UNKNOWN;

	/* If write protect is asserted, that should have worked. */
	if ((resp.flags & EC_FLASH_PROTECT_GPIO_ASSERTED) &&
	    !(resp.flags & EC_FLASH_PROTECT_RW_NOW))
		return VBERROR_UNKNOWN;

	return VBERROR_SUCCESS;
}

#else  /* CONFIG_MKBP */

/* Stub implementation for ECs which don't support MKBP */

VbError_t VbExEcRunningRW(int *in_rw)
{
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcJumpToRW(void)
{
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcStayInRO(void)
{
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcHashRW(const uint8_t **hash, int *hash_size)
{
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcUpdateRW(const uint8_t  *image, int image_size)
{
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcProtectRW(void)
{
	return VBERROR_UNKNOWN;
}

#endif  /* CONFIG_MKBP */

VbError_t VbExEcGetExpectedRW(enum VbSelectFirmware_t select,
			      const uint8_t **image, int *image_size)
{
	struct twostop_fmap fmap;
	uint32_t offset;
	firmware_storage_t file;
	uint8_t *buf;
	int size;

	*image = NULL;
	*image_size = 0;

	cros_fdtdec_flashmap(gd->fdt_blob, &fmap);
	switch (select) {
	case VB_SELECT_FIRMWARE_A:
		offset = fmap.readwrite_a.ec_bin.offset;
		size = fmap.readwrite_a.ec_bin.length;
		break;
	case VB_SELECT_FIRMWARE_B:
		offset = fmap.readwrite_b.ec_bin.offset;
		size = fmap.readwrite_b.ec_bin.length;
		break;
	default:
		VBDEBUG("Unrecognized EC firmware requested.\n");
		return VBERROR_UNKNOWN;
	}

	if (firmware_storage_open_spi(&file)) {
		VBDEBUG("Failed to open firmware storage.\n");
		return VBERROR_UNKNOWN;
	}

	VBDEBUG("EC-RW image offset %d size %d.\n", offset, size);

	/* Sanity-check; we don't expect EC images > 1MB */
	if (size <= 0 || size > 0x100000) {
		VBDEBUG("EC image has bogus size.\n");
		return VBERROR_UNKNOWN;
	}

	/*
	 * Note: this leaks memory each call, since we don't track the pointer
	 * and the caller isn't expecting to free it.
	 */
	buf = malloc(size);
	if (!buf) {
		VBDEBUG("Failed to allocate space for the EC RW image.\n");
		return VBERROR_UNKNOWN;
	}

	if (file.read(&file, offset, size, BT_EXTRA(buf))) {
		free(buf);
		VBDEBUG("Failed to read the EC from flash.\n");
		return VBERROR_UNKNOWN;
	}

	file.close(&file);

	*image = buf;
	*image_size = size;
	return VBERROR_SUCCESS;
}
