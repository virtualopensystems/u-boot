/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Debug commands for Chrome OS recovery mode firmware */

#include <common.h>
#include <command.h>
#include <fat.h>
#include <lcd.h>
#include <malloc.h>
#include <usb.h> /* for wait_ms() */
#include <chromeos/common.h>
#include <chromeos/firmware_storage.h>
#include <chromeos/load_firmware_helper.h>
#include <chromeos/load_kernel_helper.h>
#include <chromeos/gbb_bmpblk.h>
#include <chromeos/gpio.h>
#include <chromeos/os_storage.h>
#include <chromeos/vboot_nvstorage_helper.h>

#include <bmpblk_header.h>
#include <gbb_header.h>
#include <load_kernel_fw.h>
#include <vboot_nvstorage.h>

DECLARE_GLOBAL_DATA_PTR;

#define PREFIX "cros_rec: "

#ifdef VBOOT_DEBUG
#define WARN_ON_FAILURE(action) do { \
	int return_code = (action); \
	if (return_code != 0) \
		VBDEBUG(PREFIX "%s failed, returning %d\n", \
				#action, return_code); \
} while (0)
#else
#define WARN_ON_FAILURE(action) action
#endif

#define WAIT_MS_BETWEEN_PROBING	400
#define WAIT_MS_SHOW_ERROR	2000

#define STACK_MARGIN		256
#define MAX_EXCLUDED_REGIONS	16

/* This is the data structure of the to-be-cleared memory region list. */
struct MemRegion {
	uint32_t start;
	uint32_t end;
} g_memory_region, g_excluded_regions[MAX_EXCLUDED_REGIONS];
int g_excluded_size = 0;

uint8_t *g_gbb_base = NULL;
uint64_t g_gbb_size = 0;
int g_is_dev = 0;
ScreenIndex g_cur_scr = SCREEN_BLANK;

static int write_log(void)
{
	/* TODO: Implement it when Chrome OS firmware logging is ready. */
	return 0;
}

/*
 * Initializes the memory region that needs to be cleared.
 */
static void init_mem_region(uint32_t start, uint32_t end)
{
	g_memory_region.start = start;
	g_memory_region.end = end;
	g_excluded_size = 0;
}

/*
 * Excludes a memory region from the to-be-cleared region.
 * The function returns 0 on success; otherwise -1.
 */
static int exclude_mem_region(uint32_t start, uint32_t end)
{
	int i = g_excluded_size;

	if (g_excluded_size >= MAX_EXCLUDED_REGIONS) {
		VBDEBUG(PREFIX "the number of excluded regions reaches"
				"the maximum: %d\n", MAX_EXCLUDED_REGIONS);
		return -1;
	}

	while (i > 0 && g_excluded_regions[i - 1].start > start) {
		g_excluded_regions[i].start = g_excluded_regions[i - 1].start;
		g_excluded_regions[i].end = g_excluded_regions[i - 1].end;
		i--;
	}
	g_excluded_regions[i].start = start;
	g_excluded_regions[i].end = end;
	g_excluded_size++;

	return 0;
}

static void zero_mem(uint32_t start, uint32_t end)
{
	if (end > start) {
		VBDEBUG(PREFIX "\t[0x%08x, 0x%08x)\n", start, end);
		memset((void *)start, '\0', (size_t)(end - start));
	}
}

/*
 * Clears the memory not in excluded regions.
 */
static void clear_mem_regions(void)
{
	int i;
	uint32_t addr = g_memory_region.start;

	VBDEBUG(PREFIX "clear memory regions:\n");
	for (i = 0; i < g_excluded_size; ++i) {
		zero_mem(addr, g_excluded_regions[i].start);
		if (g_excluded_regions[i].end > addr)
			addr = g_excluded_regions[i].end;
	}
	zero_mem(addr, g_memory_region.end);
}

#ifdef TEST_CLEAR_MEM_REGIONS
static int test_clear_mem_regions(void)
{
	int i;
	char s[10] = "ABCDEFGHIJ";
	char r[10] = "\0BCDEFGH\0J";
	uint32_t base = (uint32_t)s;

	init_mem_region(base, base + 10);
	/* Result: ---------- */
	exclude_mem_region(base + 1, base + 2);
	/* Result: -B-------- */
	exclude_mem_region(base + 5, base + 7);
	/* Result: -B---FG--- */
	exclude_mem_region(base + 2, base + 3);
	/* Result: -BC--FG--- */
	exclude_mem_region(base + 9, base + 10);
	/* Result: -BC--FG--J */
	exclude_mem_region(base + 4, base + 6);
	/* Result: -BC-EFG--J */
	exclude_mem_region(base + 3, base + 5);
	/* Result: -BCDEFG--J */
	exclude_mem_region(base + 2, base + 8);
	/* Result: -BCDEFGH-J */
	clear_mem_regions();
	for (i = 0; i < 10; ++i) {
		if (s[i] != r[i]) {
			VBDEBUG(PREFIX "test_clear_mem_regions FAILED!\n");
			return -1;
		}
	}
	return 0;
}
#endif

/*
 * Clears the memory regions that the recovery firmware not used.
 * We do that to prevent cold boot attacks.
 */
static void clear_ram_not_in_use(void)
{
	init_mem_region(0, PHYS_SDRAM_1_SIZE);

	/* Excludes the firmware text + data + bss regions. */
	exclude_mem_region(TEXT_BASE,
			TEXT_BASE + (_bss_end - _armboot_start));

	/* TODO: Remove the GBB exclusion when we store it in SPI flash. */
	exclude_mem_region(TEXT_BASE + CONFIG_OFFSET_GBB,
			TEXT_BASE + CONFIG_OFFSET_GBB + CONFIG_LENGTH_GBB);

	/* Excludes the global data. */
	exclude_mem_region((uint32_t)gd, (uint32_t)gd + sizeof(*gd));
	exclude_mem_region((uint32_t)gd->bd,
			(uint32_t)gd->bd + sizeof(*gd->bd));

	/* Excludes the whole heap. */
	exclude_mem_region(CONFIG_STACKBASE - CONFIG_SYS_MALLOC_LEN,
			CONFIG_STACKBASE);

	/* TODO: Should excludes the in-used stack instead of the whole. */
	exclude_mem_region(CONFIG_STACKBASE,
			CONFIG_STACKBASE + CONFIG_STACKSIZE);

	/* Excludes the framebuffer. */
	exclude_mem_region(gd->fb_base, gd->fb_base + panel_info.vl_col *
			panel_info.vl_row * NBITS(panel_info.vl_bpix) / 8);

	clear_mem_regions();
}

static int init_gbb_in_ram(void)
{
	firmware_storage_t file;
	void *gbb_base = NULL;

	if (firmware_storage_init(&file)) {
		VBDEBUG(PREFIX "init firmware storage failed\n");
		return -1;
	}

	if (load_gbb(&file, &gbb_base, &g_gbb_size)) {
		VBDEBUG(PREFIX "Unable to load gbb\n");
		return -1;
	}

	g_gbb_base = gbb_base;
	return 0;
}

static int clear_screen(void)
{
	g_cur_scr = SCREEN_BLANK;
	return lcd_clear();
}

static int show_screen(ScreenIndex scr)
{
	if (g_cur_scr == scr) {
		/* No need to update. Do nothing and return success. */
		return 0;
	} else {
		g_cur_scr = scr;
		return display_screen_in_bmpblk(g_gbb_base, scr);
	}
}

int do_cros_rec(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint64_t boot_flags = BOOT_FLAG_RECOVERY;

	WARN_ON_FAILURE(write_log());

#ifdef TEST_CLEAR_MEM_REGIONS
	test_clear_mem_regions();
#endif

	clear_ram_not_in_use();

	clear_recovery_request();

	WARN_ON_FAILURE(init_gbb_in_ram());

	g_is_dev = is_developer_mode_gpio_asserted();
	if (g_is_dev)
		boot_flags |= BOOT_FLAG_DEVELOPER;

	if (!g_is_dev) {
		/* Wait for user to unplug SD card and USB storage device */
		while (is_any_storage_device_plugged(NOT_BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_MODE);
			wait_ms(WAIT_MS_BETWEEN_PROBING);
		}
	}

	for (;;) {
		/* Wait for user to plug in SD card or USB storage device */
		while (!is_any_storage_device_plugged(BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_MISSING_OS);
			wait_ms(WAIT_MS_BETWEEN_PROBING);
		}

		clear_screen();

		load_and_boot_kernel((void*) g_gbb_base, g_gbb_size,
				boot_flags);

		while (is_any_storage_device_plugged(NOT_BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_NO_OS);
			wait_ms(WAIT_MS_SHOW_ERROR);
		}
	}

	/* This point is never reached */
	return 0;
}

U_BOOT_CMD(cros_rec, 1, 1, do_cros_rec, "recovery mode firmware", NULL);