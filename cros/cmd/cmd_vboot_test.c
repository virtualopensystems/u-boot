/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/*
 * Debug commands for testing basic verified-boot related utilities.
 */

#include <common.h>
#include <command.h>
#include <chromeos/cros_gpio.h>
#include <chromeos/fdt_decode.h>
#include <chromeos/firmware_storage.h>
#include <chromeos/memory_wipe.h>
#include <vboot_api.h>

/*
 * TODO: Pick a better region for test instead of GBB.
 * We now test the region of GBB.
 */
#define TEST_FW_START		0xc1000
#define DEFAULT_TEST_FW_LENGTH	0x1000

DECLARE_GLOBAL_DATA_PTR;

static int do_vboot_test_fwrw(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	int ret = 0;
	firmware_storage_t file;
	uint32_t test_length, i;
	uint8_t *original_buf, *target_buf, *verify_buf;
	uint64_t t0, t1;

	switch (argc) {
	case 1:  /* if no argument given, use the default length */
		test_length = DEFAULT_TEST_FW_LENGTH;
		break;
	case 2:  /* use argument */
		test_length = simple_strtoul(argv[1], NULL, 16);
		if (!test_length) {
			VbExDebug("The first argument is not a number!\n");
			return cmd_usage(cmdtp);
		}
		break;
	default:
		return cmd_usage(cmdtp);
	}

	/* Allocate the buffer and fill the target test pattern. */
	original_buf = VbExMalloc(test_length);
	target_buf = VbExMalloc(test_length);
	verify_buf = VbExMalloc(test_length);

	/* Fill the target test pattern. */
	for (i = 0; i < test_length; i++)
		target_buf[i] = i & 0xff;

	/* Open firmware storage device. */
	if (firmware_storage_open_spi(&file)) {
		VbExDebug("Failed to open firmware device!\n");
		return 1;
	}

	t0 = VbExGetTimer();
	if (file.read(&file, TEST_FW_START, test_length, original_buf)) {
		VbExDebug("Failed to read firmware!\n");
		goto out;
	}
	t1 = VbExGetTimer();
	VbExDebug("test_fwrw: fw_read, length: %#x, time: %llu\n",
			test_length, t1 - t0);

	t0 = VbExGetTimer();
	ret = file.write(&file, TEST_FW_START, test_length, target_buf);
	VbExDebug("file.write returned\n");
	if (ret) {
		VbExDebug("Failed to write firmware!\n");
		ret = 1;
	} else {
		/* Read back and verify the data. */
		VbExDebug("start read again\n");
		file.read(&file, TEST_FW_START, test_length, verify_buf);
		VbExDebug("start memcmp\n");
		if (memcmp(target_buf, verify_buf, test_length) != 0) {
			VbExDebug("Verify failed. The target data wrote "
				  "wrong.\n");
			ret = 1;
		}
	}
	t1 = VbExGetTimer();
	VbExDebug("test_fwrw: fw_write, length: %#x, time: %llu\n",
			test_length, t1 - t0);

	 /* Write the original data back. */
	if (file.write(&file, TEST_FW_START, test_length, original_buf)) {
		VbExDebug("Failed to write the original data back. The "
				"firmware may now be corrupt.\n");

	}

out:
	file.close(&file);

	VbExFree(original_buf);
	VbExFree(target_buf);
	VbExFree(verify_buf);

	if (ret == 0)
		VbExDebug("Read and write firmware test SUCCESS.\n");

	return ret;
}

static int do_vboot_test_memwipe(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	memory_wipe_t wipe;
	char s[] = "ABCDEFGHIJ";
	const char r[] = "\0BCDEFGH\0J";
	const size_t size = strlen(s);
	uintptr_t base = (uintptr_t)s;

	memory_wipe_init(&wipe, base, base + size);
	/* Result: ---------- */
	memory_wipe_exclude(&wipe, base + 1, base + 2);
	/* Result: -B-------- */
	memory_wipe_exclude(&wipe, base + 5, base + 7);
	/* Result: -B---FG--- */
	memory_wipe_exclude(&wipe, base + 2, base + 3);
	/* Result: -BC--FG--- */
	memory_wipe_exclude(&wipe, base + 9, base + 10);
	/* Result: -BC--FG--J */
	memory_wipe_exclude(&wipe, base + 4, base + 6);
	/* Result: -BC-EFG--J */
	memory_wipe_exclude(&wipe, base + 3, base + 5);
	/* Result: -BCDEFG--J */
	memory_wipe_exclude(&wipe, base + 2, base + 8);
	/* Result: -BCDEFGH-J */
	memory_wipe_execute(&wipe);

	if (memcmp(s, r, size)) {
		VbExDebug("Failed to wipe the expected regions!\n");
		return 1;
	}

	VbExDebug("Memory wipe test SUCCESS!\n");
	return 0;
}

static int do_vboot_test_gpio(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	void *fdt_ptr = (void *)gd->blob;
	cros_gpio_t gpio;
	int i;

	for (i = 0; i < CROS_GPIO_MAX_GPIO; i++) {
		if (cros_gpio_fetch(i, fdt_ptr, &gpio)) {
			VbExDebug("Failed to fetch GPIO, %d!\n", i);
			return 1;
		}
		if (cros_gpio_dump(&gpio)) {
			VbExDebug("Failed to dump GPIO, %d!\n", i);
			return 1;
		}
	}
	return 0;
}

static int do_vboot_test_all(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	int ret = 0;

	ret |= do_vboot_test_fwrw(cmdtp, flag, argc, argv);
	ret |= do_vboot_test_memwipe(cmdtp, flag, argc, argv);
	ret |= do_vboot_test_gpio(cmdtp, flag, argc, argv);

	return ret;
}

static cmd_tbl_t cmd_vboot_test_sub[] = {
	U_BOOT_CMD_MKENT(all, 0, 1, do_vboot_test_all, "", ""),
	U_BOOT_CMD_MKENT(fwrw, 0, 1, do_vboot_test_fwrw, "", ""),
	U_BOOT_CMD_MKENT(memwipe, 0, 1, do_vboot_test_memwipe, "", ""),
	U_BOOT_CMD_MKENT(gpio, 0, 1, do_vboot_test_gpio, "", ""),
};

static int do_vboot_test(cmd_tbl_t *cmdtp,
		int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_vboot_test_sub[0],
			ARRAY_SIZE(cmd_vboot_test_sub));
	if (c)
		return c->cmd(c, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(vboot_test, CONFIG_SYS_MAXARGS, 1, do_vboot_test,
	"Perform tests for basic vboot related utilities",
	"all - perform all tests\n"
	"vboot_test fwrw [length] - test the firmware read/write\n"
	"vboot_test memwipe - test the memory wipe functions\n"
	"vboot_test gpio - print the status of gpio\n"
);

