/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <lcd.h>
#include <malloc.h>
#include <video.h>
#include <asm/unaligned.h>
#include <cros/cros_fdtdec.h>
#include <cros/cros_splash.h>
#include <cros/firmware_storage.h>
#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAGIC_SIZE 8
#define MAGIC_NUMBER "_splash_"
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

struct cros_splash_blob_header {
	uint8_t magic[MAGIC_SIZE];
	uint32_t version_major;
	uint32_t version_minor;
	uint32_t num_images;
} __packed;

static void *load_cros_splash_blob(const void *blob, size_t *size_ptr)
{
	struct twostop_fmap fmap;
	firmware_storage_t file;
	uint32_t offset, length;
	void *cros_splash_blob = NULL;

	if (cros_fdtdec_flashmap(gd->fdt_blob, &fmap))
		return NULL;
	offset = fmap.readonly.cros_splash.offset;
	length = fmap.readonly.cros_splash.length;
	if (!length)
		return NULL;

	cros_splash_blob = malloc(length);
	if (!cros_splash_blob)
		return NULL;

	if (firmware_storage_open_spi(&file)) {
		free(cros_splash_blob);
		return NULL;
	}

	if (file.read(&file, offset, length, BT_EXTRA(cros_splash_blob))) {
		free(cros_splash_blob);
		file.close(&file);
		return NULL;
	}

	file.close(&file);

	*size_ptr = length;
	return cros_splash_blob;
}

static int display_bitmap(ulong bitmap, int x, int y)
{
#ifdef CONFIG_LCD
	return lcd_display_bitmap(bitmap, x, y);
#elif defined(CONFIG_VIDEO)
	return video_display_bitmap(bitmap, x, y);
#else
	return -1;
#endif
}

int cros_splash_display(unsigned int index)
{
	static void *cros_splash_blob;
	static size_t cros_splash_blob_size;

	struct cros_splash_blob_header *header;
	uint32_t *offsets, *image_sizes;
	void *image, *lzma_images;
	size_t lzma_image_offset, lzma_image_size;
	SizeT image_size;
	int err, num_images;

	if (!cros_splash_blob)
		cros_splash_blob = load_cros_splash_blob(gd->fdt_blob,
				&cros_splash_blob_size);
	if (!cros_splash_blob)
		return -1;

	/* sanity check */
	if (cros_splash_blob_size < sizeof(*header))
		return -1;
	header = (struct cros_splash_blob_header *)cros_splash_blob;
	if (memcmp(header->magic, MAGIC_NUMBER, MAGIC_SIZE) ||
	    get_unaligned_be32(&header->version_major) != VERSION_MAJOR ||
	    get_unaligned_be32(&header->version_minor) != VERSION_MINOR)
		return -1;

	num_images = get_unaligned_be32(&header->num_images);
	if (index >= num_images)
		return -1;

	offsets = (uint32_t *)(cros_splash_blob + sizeof(*header));
	image_sizes = offsets + num_images;
	lzma_images = image_sizes + num_images;
	if (lzma_images > cros_splash_blob + cros_splash_blob_size)
		return -1;

	image_size = get_unaligned_be32(image_sizes + index);
	image = malloc(image_size);
	if (!image)
		return -1;

	if (index > 0)
		lzma_image_offset = get_unaligned_be32(offsets + index - 1);
	else
		lzma_image_offset = 0;
	lzma_image_size = get_unaligned_be32(offsets + index);
	lzma_image_size -= lzma_image_offset;
	if (lzma_images + lzma_image_offset + lzma_image_size >
			cros_splash_blob + cros_splash_blob_size)
		goto err;

	err = lzmaBuffToBuffDecompress(image, &image_size,
			lzma_images + lzma_image_offset, lzma_image_size);
	if (err != SZ_OK)
		goto err;

	err = display_bitmap((ulong)image, 0, 0);
	if (err)
		goto err;

	free(image);
	return 0;

err:
	free(image);
	return -1;
}

static int do_cros_splash(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	unsigned long index;

	if (argc != 2)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[1], 10, &index))
		return CMD_RET_USAGE;

	if (cros_splash_display(index))
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_CMD(cros_splash, 2, 1, do_cros_splash, "display splash image",
		"index\n    - display splash image <index>\n");
