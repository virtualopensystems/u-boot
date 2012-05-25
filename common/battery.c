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
#include <battery.h>
#include <smartbat.h>
#include <tps65090.h>

static const char *status_name[BATTERY_STATE_COUNT] = {
	"unknown",
	"missing",
	"idle",
	"discharging",
	"charging",
	"charged",
	"discharged",
};

static const char *action_name[BATTERY_ACT_COUNT] = {
	"none",
	"start_charge",
	"stop_charge",
	"disconnect",
};

/**
 * Calculate the battery status based on supplied input
 *
 * Work out the status based on the supplied information
 *
 * @param battery_status	Current value of battery status word
 * @param charging		Charging indicator (0/1 or -1 for error)
 * @return the battery status
 */
static enum charge_status_t convert_battery_status(int battery_status,
						   int charging)
{
	if (battery_status == -1)
		return BATTERY_MISSING;
	if (battery_status & SMARTBAT_ST_DISCHARGED)
		return BATTERY_DISCHARGED;
	if (battery_status & SMARTBAT_ST_CHARGED)
		return BATTERY_CHARGED;
	if (battery_status & SMARTBAT_ST_DISCHARGING)
		return BATTERY_DISCHARGING;
	if (charging < 0)
		return BATTERY_UNKNOWN;
	return charging ? BATTERY_CHARGING : BATTERY_IDLE;
}

#if defined(CONFIG_SMARTBAT_POWER) && defined(CONFIG_TPS65090_POWER)
enum charge_status_t battery_get_status(void)
{
	int battery_status;
	int charging;

	battery_status = smartbat_get_status();
	charging = tps65090_get_charging();
	return convert_battery_status(battery_status, charging);
}

int battery_get_temperature(int *milli_degrees_c)
{
	return smartbat_get_temperature(milli_degrees_c);
}

int battery_get_charge_state(struct battery_charge_state *state)
{
	struct battery_info info;

	state->seconds = get_timer(state->start_time) / 1000;

	state->req_action = BATTERY_ACT_NONE;
	state->battery_status = smartbat_get_status();
	state->charging = tps65090_get_charging();
	state->charger_status = tps65090_get_status();
	if (state->charging < 0)
		return BATTERY_ERR_UNKNOWN_STATE;

	if (state->battery_status >= 0) {
		if (battery_get_info(&info))
			return BATTERY_ERR_CHARGER_COMMS;

		state->percent_charge = info.relative_charge;
		state->average_current_ma = info.average_current_ma;
		if (smartbat_get_temperature(&state->milli_degrees_c))
			return BATTERY_ERR_CHARGER_COMMS;
	} else {
		state->percent_charge = 0;
		state->average_current_ma = 0;
	}

	return 0;
}

int battery_get_info(struct battery_info *info)
{
	return smartbat_get_info(info);
}

int battery_set_charge_enable(int enable)
{
	return tps65090_set_charge_enable(enable);
}

int battery_init(void)
{
	return smartbat_init();
}
#endif

int battery_charge_init(struct battery_charge_state *state)
{
	memset(state, '\0', sizeof(*state));
	state->start_time = get_timer(0);

	return 0;
}

enum battery_action_t battery_calculate_action(
		struct battery_charge_state *state,
		const struct battery_charge_config *config)
{
	enum battery_action_t action = BATTERY_ACT_STOP_CHARGE;

	state->status = convert_battery_status(state->battery_status,
					       state->charging);
	debug("%s: status %#04x\n", __func__, state->status);

	if (state->status != BATTERY_MISSING) {
		if (state->battery_status & SMARTBAT_ST_STOP_DISCHARGE)
			state->req_action = BATTERY_ACT_DISCONNECT;
		else if (state->battery_status & (SMARTBAT_ST_OVER_TEMP
				| SMARTBAT_ST_STOP_CHARGE
				| SMARTBAT_ST_OVER_CHARGE))
			state->req_action = BATTERY_ACT_STOP_CHARGE;
	}

	/* Charger error if we can't talk to charger, or it has problem */
	state->charger_error = state->charger_status &
		(TPS65090_ST1_OTC | TPS65090_ST1_OCC);

	/* If the battery is requesting an action, take it */
	switch (state->req_action) {
	case BATTERY_ACT_NONE:
	case BATTERY_ACT_START_CHARGE:
	case BATTERY_ACT_COUNT:
		/* ignore these */
		break;
	case BATTERY_ACT_STOP_CHARGE:
		/* Stop charging if we are charging */
		if (state->charging > 0)
			return BATTERY_ACT_STOP_CHARGE;
		return BATTERY_ACT_NONE;
	case BATTERY_ACT_DISCONNECT:
		/* Disconnect, regardless of current state */
		return BATTERY_ACT_DISCONNECT;
	}

	/* If we are not charging, we are probably going to do nothing */
	if (!state->charging)
		action = BATTERY_ACT_NONE;

	/* Battery missing: stop charging */
	if (state->status == BATTERY_MISSING)
		return action;

	/* Temperature too low: stop charging */
	if (state->milli_degrees_c < config->min_temperature)
		return action;

	/* Temperature too high: disconnect */
	if (state->milli_degrees_c >= config->max_temperature)
		return BATTERY_ACT_DISCONNECT;

	/* Temperature too high: stop charging */
	if (state->milli_degrees_c >= config->max_charge_temperature) {
		if (state->charging != 0)
			return BATTERY_ACT_STOP_CHARGE;
	}

	/* Check for charger error */
	if (state->charger_error)
		return action;

	/* Can we start charging? */
	if (state->milli_degrees_c < config->max_start_temperature) {
		if (!state->charging && state->status != BATTERY_CHARGED)
			action = BATTERY_ACT_START_CHARGE;
	}

	/* Can we continue charging? */
	if (state->milli_degrees_c < config->max_charge_temperature &&
			action == BATTERY_ACT_STOP_CHARGE)
		action = BATTERY_ACT_NONE;

	return action;
}

const char *battery_get_status_name(enum charge_status_t status)
{
	assert(status >= BATTERY_UNKNOWN && status < BATTERY_STATE_COUNT);
	return status_name[status];
}

const char *battery_get_action_name(enum battery_action_t action)
{
	assert(action >= BATTERY_ACT_NONE && action < BATTERY_ACT_COUNT);
	return action_name[action];
}
