/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Define this to make sure that our assert()s will activate */
#define DEBUG

#include <common.h>
#include <battery.h>
#include <smartbat.h>

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

#define TESTEQ(a, b)						\
	if ((a) != (b)) {					\
		debug("Failure at %s:%d: %s, %d != %d\n",	\
		      __func__, __LINE__, #a " != " #b, a, b);	\
		return -1;					\
	}

#define TEST_ACTION(a)						\
	action = battery_calculate_action(state, &config);	\
	TESTEQ(action, a);

static int battery_test(struct battery_charge_state *state)
{
	enum battery_action_t action;

	state->seconds = 0;
	state->req_action = BATTERY_ACT_NONE;
	state->battery_status = SMARTBAT_ST_INITED;
	state->charging = 0;
	state->charger_status = 0;

	state->percent_charge = 50;
	state->average_current_ma = 1000;
	state->milli_degrees_c = 25000;

	/* Decide what action to take in response */
	TEST_ACTION(BATTERY_ACT_START_CHARGE);

	/* Start charging as requested, see that we keep charging */
	state->charging = 1;
	state->milli_degrees_c = 25000;
	TEST_ACTION(BATTERY_ACT_NONE);

	/* And again */
	state->milli_degrees_c = 25000;
	TEST_ACTION(BATTERY_ACT_NONE);

	/* Drop the temperature too low */
	state->milli_degrees_c = 4000;
	TEST_ACTION(BATTERY_ACT_STOP_CHARGE);
	state->charging = 0;

	/* And again */
	TEST_ACTION(BATTERY_ACT_NONE);

	/* Put the temperature above the start charge level */
	state->milli_degrees_c = 41000;
	TEST_ACTION(BATTERY_ACT_NONE);

	/* Drop it down a bit and see that we start charging */
	state->milli_degrees_c = 39000;
	TEST_ACTION(BATTERY_ACT_START_CHARGE);
	state->charging = 1;

	/* Put it back up and see that we continue charging */
	state->milli_degrees_c = 41000;
	TEST_ACTION(BATTERY_ACT_NONE);

	/* Go too high and see that we stop charging */
	state->milli_degrees_c = 56000;
	TEST_ACTION(BATTERY_ACT_STOP_CHARGE);
	state->charging = 0;

	/* Drop it down a bit and see that we start charging */
	state->milli_degrees_c = 39000;
	TEST_ACTION(BATTERY_ACT_START_CHARGE);
	state->charging = 1;

	/* Go really high and see that we disconnect */
	state->milli_degrees_c = 66000;
	TEST_ACTION(BATTERY_ACT_DISCONNECT);
	state->charging = 1;

	return 0;
}

static int do_ut_battery(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	struct battery_charge_state state;
	int err;

	printf("%s: Testing battery charging algorithms\n", __func__);
	err = battery_test(&state);
	printf("%s\n", err ? "FAILED" : "PASSED");

	return 0;
}

U_BOOT_CMD(
	ut_battery,	5,	1,	do_ut_battery,
	"Unit test of battery charging algorithms",
	""
);
