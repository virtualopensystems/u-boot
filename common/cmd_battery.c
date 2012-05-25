/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * Battery command interface
 */

#include <common.h>
#include <battery.h>
#include <asm/arch/power.h>

/* TODO(sjg@chromium.org): Move this to device tree */
static const struct battery_charge_config config = {
	/*
	 * Limit temperatures in milli-degrees celsius.
	 *
	 * We have a three maximum temperatures:
	 *    - maximum above which we will not *start* charging
	 *    - maximum above which we stop charging
	 *    - absolute maximum above which we disconnect power and turn off.
	 */
	.min_temperature = 5000,		/* Minimum for charging */
	.max_start_temperature = 40000,		/* Maximum to start charging */
	.max_charge_temperature = 55000,	/* Maximum to charge */
	.max_temperature = 65000,		/* Over this we disconnect */
};

/**
 * Perform a single step of the battery state machine
 *
 * @param state		Returns resulting battery state
 * @return 0 if ok, -1 on error
 */
static int battery_charge_step(struct battery_charge_state *state,
			       const struct battery_charge_config *config)
{
	int err;

	/* Work out the current state of affairs */
	err = battery_get_charge_state(state);
	if (err) {
		debug("%s: Cannot read battery state\n", __func__);
		return err;
	}

	/* Decide what action to take in response */
	state->action = battery_calculate_action(state, config);

	/* Perform that action */
	switch (state->action) {
	case BATTERY_ACT_START_CHARGE:
		err = battery_set_charge_enable(1);
		debug("Charging enabled\n");
		break;
	case BATTERY_ACT_STOP_CHARGE:
		err = battery_set_charge_enable(0);
		debug("Charging disabled\n");
		break;
	case BATTERY_ACT_DISCONNECT:
		printf("Critical battery error: powering off\n");
		udelay(5 * 1000);
		power_shutdown();	/* Does not return */
		break;
	case BATTERY_ACT_NONE:
	case BATTERY_ACT_COUNT:
		break;
	}
	if (err) {
		debug("%s: Cannot change charge state\n", __func__);
		return err;
	}

	return 0;
}

static int battery_charge_report(struct battery_charge_state *state)
{
	printf("%us: %s", state->seconds,
	       battery_get_status_name(state->status));
	if (state->status != BATTERY_MISSING) {
		printf(" %d%% %dmA %d.%ldC (status bat=%#04x, tps=%#02x)",
		       state->percent_charge,
		       state->average_current_ma,
		       state->milli_degrees_c / 1000,
		       (abs(state->milli_degrees_c) % 1000) / 100,
		       state->battery_status, state->charger_status);
		if (state->action)
			printf(", %s", battery_get_action_name(state->action));
	}
	puts("\n");

	return 0;
}

static int do_battery(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	const char *cmd;
	int ret = 0;

	if (argc < 2)
		return CMD_RET_USAGE;
	cmd = argv[1];
	argc -= 2;
	argv += 2;
	if (!strncmp(cmd, "info", 3)) {
		struct battery_info info;

		if (battery_get_info(&info)) {
			printf("Error getting battery info\n");
		} else {
			printf("Manufacturer: %s\n", info.manuf_name);
			printf("Device: %s\n", info.device_name);
			printf("Chemistry: %s\n", info.device_chem);
			printf("Serial number: %d\n", info.serial_num);
			printf("Design capacity (mAhr): %d\n",
			       info.design_cap_mah);
			printf("Design voltage (mV): %d\n",
			       info.design_voltage_mv);
			printf("Voltage (mV): %d\n", info.voltage_mv);
			printf("Average current (mA): %u\n",
			       info.average_current_ma);
			printf("Relative charge (%%): %u\n",
			       info.relative_charge);
			printf("Battery status: %04x\n", info.battery_status);
		}
	} else if (!strncmp(cmd, "init", 1)) {
		ret = battery_init();
	} else if (!strncmp(cmd, "charge", 1)) {
		if (!argc) {
			int status;

			status = battery_get_status();
			printf("%s\n", battery_get_status_name(status));
		} else {
			int enable = simple_strtoul(*argv, NULL, 10);

			ret = battery_set_charge_enable(enable);
		}
	} else if (!strncmp(cmd, "loop", 1)) {
		struct battery_charge_state state;
		unsigned long start;

		printf("Entering charging loop, press <space> to stop\n");
		ret = battery_charge_init(&state);
		start = get_timer(0);
		while (!ret && !tstc()) {
			ret = battery_charge_step(&state, &config);
			if (!ret)
				ret = battery_charge_report(&state);
			if (ret) {
				printf("(battery error %d)\n", ret);
				ret = 0;
			}
			while (!tstc() && get_timer(start) < 10 * 1000)
				;
			start += 10 * 1000;
		}
		if (!ret)
			printf("Charge loop complete (can power down)\n");
	} else if (!strncmp(cmd, "temperature", 1)) {
		int m_degrees_c;

		ret = battery_get_temperature(&m_degrees_c);
		printf("Battery temperature %u.%lu\n", m_degrees_c / 1000,
		       (abs(m_degrees_c) % 1000) / 100);
	} else {
		return CMD_RET_USAGE;
	}
	if (ret)
		printf("Battery error %d\n", ret);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	battery,	5,	1,	do_battery,
	"Battery utility command",
	" - perform battery operations\n"
	"charge [<0/1>]     - See charge state, or turn charging on / off\n"
	"info               - Display battery information\n"
	"temperature        - Display battery temperature\n"
	"loop               - Go into battery charge loop"
);
