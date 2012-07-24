/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/*
 * Debug commands for testing Verify Boot Export APIs that are implemented by
 * firmware and exported to vboot_reference. Some of the tests are automatic
 * but some of them are manual.
 */

#include <battery.h>
#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <i2c.h>
#include <smartbat.h>
#include <tps65090.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* #define TEST_BATTERY */

static int do_cros_test_i2c(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
#ifdef CONFIG_TPS65090_POWER
	int fet_id = 2;
	int ret = 0;
	int pass;

# ifdef TEST_BATTERY
	struct battery_info base_info, info;

	if (smartbat_init()) {
		printf("Cannot init smart battery\n");
		return 1;
	}
# endif
	if (tps65090_init()) {
		printf("Cannot init tps65090\n");
		return 1;
	}
# ifdef TEST_BATTERY
	if (smartbat_get_info(&base_info)) {
		printf("Cannot get battery info\n");
		return 1;
	}
# endif
	/* Generate continuous i2c traffic and make sure all is well */
	for (pass = 0; pass < 100000; pass++) {
		/* Do some things on the bus, first with TPSCHROME */
		ret = tps65090_fet_enable(fet_id);
		if (ret)
			break;
		if (!tps65090_fet_is_enabled(fet_id)) {
			ret = -1;
			break;
		}

		ret = tps65090_fet_disable(fet_id);
		if (ret)
			break;
		if (tps65090_fet_is_enabled(fet_id)) {
			ret = -1;
			break;
		}
# ifdef TEST_BATTERY
		/* Now have a look at the battery */
		if (smartbat_get_info(&info)) {
			printf("Pass %d: Battery read error\n", pass);
			continue;
		}

		if (0 == memcmp(&base_info, &info, sizeof(info))) {
			puts("Battery info comparison failed\n");
			ret = -1;
			break;
		}
# endif
		if (ctrlc()) {
			puts("Test terminated by ctrl-c\n");
			break;
		}

		/* mdelay(1); */
	}

	if (ret)
		printf("Error detected on pass %d\n", pass);
#endif /* CONFIG_TPS65090_POWER */
	return 0;
}

#ifdef CONFIG_DRIVER_S3C24X0_I2C
#include <asm/arch/pinmux.h>

static int setup_i2c(int *nodep)
{
	int node;
	int err;

	node = fdtdec_find_alias_node(gd->fdt_blob, "i2c4");
	if (node < 0) {
		printf("Error: Cannot find fdt node\n");
		return -1;
	}

	if (board_i2c_claim_bus(node)) {
		printf("Error: Cannot claim bus\n");
		return -1;
	}
	err = gpio_request(GPIO_A20, "i2creset");
	err |= gpio_request(GPIO_A21, "i2creset");
	err |= gpio_direction_output(GPIO_A20, 0);
	err |= gpio_direction_output(GPIO_A21, 0);
	if (err) {
		printf("Error: Could not set up GPIOs\n");
		return -1;
	}
	*nodep = node;

	return 0;
}

static int restore_i2c(int node)
{
	int err;

	err = gpio_direction_output(GPIO_A20, 1);
	err |= gpio_direction_output(GPIO_A21, 1);
	if (err) {
		printf("Error: Could not restore GPIOs\n");
		return -1;
	}

	if (i2c_reset_port_fdt(gd->fdt_blob, node)) {
		printf("Error: Could not reset I2C after operation\n");
		return -1;
	}

	if (exynos_pinmux_config(PERIPH_ID_I2C4, 0)) {
		printf("Error: Could not restore I2C\n");
		return -1;
	}
	board_i2c_release_bus(node);

	return 0;
}

static int do_cros_test_i2creset(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int node;

	printf("Holding I2C bus 4 low for 15 seconds\n");
	if (setup_i2c(&node))
		return 1;

	mdelay(15 * 1000);
	if (restore_i2c(node))
		return 1;

	printf("   - done, I2C restored\n");

	return 0;
}

/**
 * Return a 1-bit pseudo-random number related to the supplied seed
 *
 * @param seed		Seed value (any integer)
 * @return pseudo-random number (0 or 1)
 */
static int get_fiddle_value(int seed)
{
	int value = 0;
	int i;

	for (i = 0; i < 32; i++)
		value ^= (seed >> i) & 1;

	return value;
}

static int do_cros_test_i2cfiddle(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int node, i;

	printf("Generate random transitions on I2C bus 4\n");
	if (setup_i2c(&node))
		return 1;

	for (i = 0; i < 100; i++) {
		gpio_set_value(GPIO_A20, get_fiddle_value(i));
		udelay(1);
		gpio_set_value(GPIO_A21, get_fiddle_value(100 - i));
		udelay(1);
	}

	if (restore_i2c(node))
		return 1;
	printf("   - done, I2C restored\n");

	return 0;
}
#endif

static int do_cros_test_all(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int ret = 0;

	ret |= do_cros_test_i2c(cmdtp, flag, argc, argv);
	if (!ret)
		printf("All tests passed\n");
	return ret;
}

static cmd_tbl_t cmd_cros_test_sub[] = {
	U_BOOT_CMD_MKENT(i2c, 0, 1, do_cros_test_i2c, "", ""),
#ifdef CONFIG_DRIVER_S3C24X0_I2C
	U_BOOT_CMD_MKENT(i2creset, 0, 1, do_cros_test_i2creset, "", ""),
	U_BOOT_CMD_MKENT(i2cfiddle, 0, 1, do_cros_test_i2cfiddle, "", ""),
#endif
	U_BOOT_CMD_MKENT(all, 0, 1, do_cros_test_all, "", ""),
};

static int do_cros_test(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_cros_test_sub[0],
			ARRAY_SIZE(cmd_cros_test_sub));
	if (c)
		return c->cmd(c, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(cros_test, CONFIG_SYS_MAXARGS, 1, do_cros_test,
	"Perform tests for Chrome OS",
	"all        Run all tests\n"
	"i2c        Test i2c link with EC, and arbitration\n"
	"i2creset   Try to reset i2c bus by holding clk, data low for 15s\n"
	"i2cfiddle  Try to break TPSCHROME or the battery on i2c"
);
