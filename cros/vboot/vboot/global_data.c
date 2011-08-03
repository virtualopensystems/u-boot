/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <fdt_decode.h>
#include <chromeos/common.h>
#include <chromeos/cros_gpio.h>
#include <chromeos/gbb.h>
#include <vboot/global_data.h>

#include <gbb_header.h>
#include <vboot_api.h>

#define PREFIX		"global_data: "

DECLARE_GLOBAL_DATA_PTR;

vb_global_t *get_vboot_global(void)
{
	return (vb_global_t *)(CONFIG_VBGLOBAL_BASE);
}

int init_vboot_global(vb_global_t *global, firmware_storage_t *file)
{
	cros_gpio_t wpsw, recsw, devsw;
	struct twostop_fmap fmap;
	uint8_t frid[ID_LEN];
	GoogleBinaryBlockHeader *gbb =
			(GoogleBinaryBlockHeader *)global->gbb_data;

	global->version = VBGLOBAL_VERSION;
	memcpy(global->signature, VBGLOBAL_SIGNATURE,
			VBGLOBAL_SIGNATURE_SIZE);

	/* Gets GPIO status */
	if (cros_gpio_fetch(CROS_GPIO_WPSW, &wpsw) ||
			cros_gpio_fetch(CROS_GPIO_RECSW, &recsw) ||
			cros_gpio_fetch(CROS_GPIO_DEVSW, &devsw)) {
		VBDEBUG(PREFIX "Failed to fetch GPIO!\n");
		return 1;
        }

	if (fdt_decode_twostop_fmap(gd->blob, &fmap)) {
		VBDEBUG(PREFIX "Failed to load fmap config from fdt!\n");
		return 1;
	}

	/* Loads a minimal required part of GBB from SPI */
	if (fmap.readonly.gbb.length > GBB_MAX_LENGTH) {
		VBDEBUG(PREFIX "The GBB size declared in FDT is too big!\n");
		return 1;
	}
	global->gbb_size = fmap.readonly.gbb.length;

	if (gbb_init(global->gbb_data, file, fmap.readonly.gbb.offset)) {
		VBDEBUG(PREFIX "Failed to read GBB!\n");
		return 1;
	}

	if (fmap.readonly.firmware_id.length > ID_LEN) {
		VBDEBUG(PREFIX "The FWID size declared in FDT is too big!\n");
		return 1;
	}
	if (file->read(file,
			fmap.readonly.firmware_id.offset,
			fmap.readonly.firmware_id.length,
			frid)) {
		VBDEBUG(PREFIX "Failed to read frid!\n");
		return 1;
	}

	if (crossystem_data_init(&global->cdata_blob,
			&wpsw, &recsw, &devsw,
			fmap.readonly.fmap.offset,
			ACTIVE_EC_FIRMWARE_RO,
			(uint8_t *)global->gbb_data + gbb->hwid_offset,
			frid)) {
		VBDEBUG(PREFIX "Failed to init crossystem data!\n");
		return 1;
	}

	global->cdata_size = sizeof(crossystem_data_t);

	return 0;
}

int is_vboot_global_valid(vb_global_t *global)
{
	return (global && memcmp(global->signature,
		VBGLOBAL_SIGNATURE, VBGLOBAL_SIGNATURE_SIZE) == 0);
}
