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
#include <chromeos/common.h>
#include <chromeos/gbb.h>

#include <gbb_header.h>

#define PREFIX "gbb: "

int gbb_init(void *gbb, firmware_storage_t *file, uint32_t gbb_offset)
{
	GoogleBinaryBlockHeader *gbbh = (GoogleBinaryBlockHeader *)gbb;

	if (file->read(file, gbb_offset, sizeof(*gbbh), gbbh)) {
		VBDEBUG(PREFIX "failed to read GBB header\n");
		return 1;
	}

	if (file->read(file, gbb_offset + gbbh->hwid_offset,
				gbbh->hwid_size,
				gbb + gbbh->hwid_offset)) {
		VBDEBUG(PREFIX "failed to read hwid\n");
		return 1;
	}

	if (file->read(file, gbb_offset + gbbh->rootkey_offset,
				gbbh->rootkey_size,
				gbb + gbbh->rootkey_offset)) {
		VBDEBUG(PREFIX "failed to read root key\n");
		return 1;
	}

	return 0;
}

int gbb_read_bmp_block(void *gbb, firmware_storage_t *file, uint32_t gbb_offset)
{
	GoogleBinaryBlockHeader *gbbh = (GoogleBinaryBlockHeader *)gbb;

	if (file->read(file, gbb_offset + gbbh->bmpfv_offset,
				gbbh->bmpfv_size,
				gbb + gbbh->bmpfv_offset)) {
		VBDEBUG(PREFIX "failed to read bmp block\n");
		return 1;
	}

	return 0;
}

int gbb_read_recovery_key(void *gbb,
		firmware_storage_t *file, uint32_t gbb_offset)
{
	GoogleBinaryBlockHeader *gbbh = (GoogleBinaryBlockHeader *)gbb;

	if (file->read(file, gbb_offset + gbbh->recovery_key_offset,
				gbbh->recovery_key_size,
				gbb + gbbh->recovery_key_offset)) {
		VBDEBUG(PREFIX "failed to read recovery key\n");
		return 1;
	}

	return 0;
}
