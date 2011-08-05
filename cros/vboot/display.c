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
#ifdef CONFIG_LCD
#include <lcd.h>
#endif
#ifdef CONFIG_CFB_CONSOLE
#include <video.h>
#endif
#include <chromeos/common.h>
#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>

#define PRINT_MAX_ROW	20
#define PRINT_MAX_COL	80

DECLARE_GLOBAL_DATA_PTR;

struct display_callbacks {
	int (*dc_get_pixel_width) (void);
	int (*dc_get_pixel_height) (void);
	int (*dc_get_screen_columns) (void);
	int (*dc_get_screen_rows) (void);
	void (*dc_position_cursor) (unsigned col, unsigned row);
	void (*dc_puts) (const char *s);
	int (*dc_display_bitmap) (ulong, int, int);
	int (*dc_display_clear) (void);
};

static struct display_callbacks display_callbacks_ = {
#ifdef CONFIG_LCD
	.dc_get_pixel_width = lcd_get_pixel_width,
	.dc_get_pixel_height = lcd_get_pixel_height,
	.dc_get_screen_columns = lcd_get_screen_columns,
	.dc_get_screen_rows = lcd_get_screen_rows,
	.dc_position_cursor = lcd_position_cursor,
	.dc_puts = lcd_puts,
	.dc_display_bitmap = lcd_display_bitmap,
	.dc_display_clear = lcd_clear,

#else
	.dc_get_pixel_width = video_get_pixel_width,
	.dc_get_pixel_height = video_get_pixel_height,
	.dc_get_screen_columns = video_get_screen_columns,
	.dc_get_screen_rows = video_get_screen_rows,
	.dc_position_cursor = video_position_cursor,
	.dc_puts = video_puts,
	.dc_display_bitmap = video_display_bitmap,
	.dc_display_clear = video_clear
#endif
};

VbError_t VbExDisplayInit(uint32_t *width, uint32_t *height)
{
	*width = display_callbacks_.dc_get_pixel_width();
	*height = display_callbacks_.dc_get_pixel_height();
	return VBERROR_SUCCESS;
}

VbError_t VbExDisplayBacklight(uint8_t enable)
{
	/* TODO(waihong@chromium.org) Implement it later */
	return VBERROR_SUCCESS;
}

/* Print the message on the center of LCD. */
static void print_on_center(const char *message)
{
	int i, room_before_text;
	int screen_size = display_callbacks_.dc_get_screen_columns() *
		display_callbacks_.dc_get_screen_rows();
	int text_length = strlen(message);

	room_before_text = (screen_size - text_length) / 2;

	display_callbacks_.dc_position_cursor(0, 0);

	for (i = 0; i < room_before_text; i++)
		display_callbacks_.dc_puts(".");

	display_callbacks_.dc_puts(message);

	for (i = i + text_length; i < screen_size; i++)
		display_callbacks_.dc_puts(".");
}

VbError_t VbExDisplayScreen(uint32_t screen_type)
{
	/*
	 * Show the debug messages for development. It is a backup method
	 * when GBB does not contain a full set of bitmaps.
	 */
	switch (screen_type) {
		case VB_SCREEN_BLANK:
			/* clear the screen */
			display_clear();
			break;
		case VB_SCREEN_DEVELOPER_WARNING:
			print_on_center("developer mode warning");
			break;
		case VB_SCREEN_DEVELOPER_EGG:
			print_on_center("easter egg");
			break;
		case VB_SCREEN_RECOVERY_REMOVE:
			print_on_center("remove inserted devices");
			break;
		case VB_SCREEN_RECOVERY_INSERT:
			print_on_center("insert recovery image");
			break;
		case VB_SCREEN_RECOVERY_NO_GOOD:
			print_on_center("insert image invalid");
			break;
		default:
			VBDEBUG("Not a valid screen type: %08x.\n",
					screen_type);
			return 1;
	}
	return VBERROR_SUCCESS;
}

static uint8_t *uncompress_lzma(uint8_t *in_addr, SizeT in_size,
                                SizeT out_size)
{
	uint8_t *out_addr = VbExMalloc(out_size);
	SizeT lzma_len = out_size;
	int ret;

	ret = lzmaBuffToBuffDecompress(out_addr, &lzma_len, in_addr, in_size);
	if (ret != SZ_OK) {
		VbExFree(out_addr);
		out_addr = NULL;
	}
	return out_addr;
}

VbError_t VbExDisplayImage(uint32_t x, uint32_t y, const ImageInfo *info,
                           const void *buffer)
{
	int ret;
	uint8_t *raw_data;

	switch (info->compression) {
	case COMPRESS_NONE:
		raw_data = (uint8_t *)buffer;
		break;

	case COMPRESS_LZMA1:
		raw_data = uncompress_lzma((uint8_t *)buffer,
				(SizeT)info->compressed_size,
				(SizeT)info->original_size);
		if (!raw_data) {
			VBDEBUG("LZMA decompress failed.\n");
			return 1;
		}
		break;

	default:
		VBDEBUG("Unsupported compression format: %08x\n",
				info->compression);
		return 1;
	}

	ret = display_callbacks_.dc_display_bitmap((ulong)raw_data, x, y);

	if (info->compression == COMPRESS_LZMA1)
		VbExFree(raw_data);

	if (ret) {
		VBDEBUG("LCD display error.\n");
		return 1;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExDisplayDebugInfo(const char *info_str)
{
	display_callbacks_.dc_position_cursor(0, 0);
	display_callbacks_.dc_puts(info_str);
	return VBERROR_SUCCESS;
}

/* this function is not technically part of the vboot interface */
int display_clear(void)
{
	return display_callbacks_.dc_display_clear();
}
