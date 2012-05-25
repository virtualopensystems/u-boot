/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * Generic battery interface, used to avoid access to specific drivers from
 * generic code. We provide access to information about the battery and a
 * battery charging state machine which turns charging on / off as required
 * by the supplied charge configuration.
 *
 * At present this supports:
 *   tps65090: A PMIC / battery charger IC
 *   A smart battery, conforming to the smart battery specification
 *
 * We could generalise this to support multipler chargers/fuel gauages if
 * required.
 */

#ifndef __BATTERY_H
#define __BATTERY_H

/* The various states that the battery can be in */
enum charge_status_t {
	BATTERY_UNKNOWN,	/* Can't communicate with charger */
	BATTERY_MISSING,	/* Cannot find the battery */
	BATTERY_IDLE,		/* Nothing happening */
	BATTERY_DISCHARGING,	/* Battery is discharging */
	BATTERY_CHARGING,	/* Battery is charging */
	BATTERY_CHARGED,	/* Battery is fully charged */
	BATTERY_DISCHARGED,	/* Battery is fully discharged */

	BATTERY_STATE_COUNT,
};

/* Information about how to charge the battery, limits, etc. */
struct battery_charge_config {
	int min_temperature;		/* minimum temperature to charge */
	int max_start_temperature;	/* max temperature to start charging */
	int max_charge_temperature;	/* max temperature to charge */
	int max_temperature;		/* max temperature to allow */
};

/* Some errors we can return */
enum battery_err_t {
	BATTERY_ERR_OK,			/* no error */
	BATTERY_ERR_CHARGER_COMMS = -2,	/* cannot talk to charger */
	BATTERY_ERR_FUEL_GAUGE_COMMS = -3,	/* cannot talk to fuel guage */
	BATTERY_ERR_UNKNOWN_STATE = -4,	/* battery in unknown state */
};

/* Requested battery charge actions */
enum battery_action_t {
	BATTERY_ACT_NONE,		/* no action */
	BATTERY_ACT_START_CHARGE,	/* start charging */
	BATTERY_ACT_STOP_CHARGE,	/* stop charging */
	BATTERY_ACT_DISCONNECT,		/* disconnect power */

	BATTERY_ACT_COUNT,
};

/* Holds all the state about the battery system while charging */
struct battery_charge_state {
	unsigned start_time;		/* time that we started */
	unsigned seconds;		/* seconds since boot */

	/* Parameters we read from the system in battery_get_state() */
	unsigned percent_charge;	/* percentage changed (0-100) */
	unsigned average_current_ma;	/* average current in mA */
	int battery_status;		/* battery status, -1 if unknown */
	int charger_status;		/* charger status register, or -1 */
	int milli_degrees_c;		/* battery temperature */
	int charging;			/* -1=err, 0=!charging, 1=charging */

	/* Derived information and decisions in calculate_action() */
	int charger_error;		/* non-zero if charger has problem */
	enum charge_status_t status;	/* current charge status */
	enum battery_action_t req_action;	/* requested battery action */
	enum battery_action_t action;	/* selected battery action */
};

/* This structure holds basic information about the battery */
struct battery_info {
	char manuf_name[20];		/* Manufacturer name */
	char device_name[12];		/* Device name */
	char device_chem[10];		/* Device chemistry, e,g, 'LION' */
	unsigned serial_num;		/* Serial number */
	unsigned design_cap_mah;	/* Design capacity in mA hours */
	unsigned design_voltage_mv;	/* Design voltage in mV */
	unsigned voltage_mv;		/* Current voltage in mV */
	int average_current_ma;		/* Average current (-ve = discharge) */
	unsigned relative_charge;	/* Relative charge % (0-100) */
	unsigned battery_status;	/* Raw battery status */
};

/**
 * Get the current battery temperature
 *
 * The temperature is measured in thousandths of degrees celcius. So 25
 * degrees C will be reported as 25000.
 *
 * @param milli_degrees_c	Place to put temperature
 * @return 0 if ok. -1 on error
 */
int battery_get_temperature(int *milli_degrees_c);

/**
 * Get information about the battery system
 *
 * @param info	Place to put battery info
 * @return 0 if ok, -1 if unable to get info / communicate with battery
 */
int battery_get_info(struct battery_info *info);

/**
 * Get the name of a battery charge status value
 *
 * @param status	Battery charge status to look up
 * @return pointer to name
 */
const char *battery_get_status_name(enum charge_status_t status);

/**
 * Get the name of a battery action value
 *
 * @param status	Battery charge status to look up
 * @return pointer to name
 */
const char *battery_get_action_name(enum battery_action_t action);

/**
 * Read the battery status
 *
 * We calculate this based on input from smartbat and tps65090. If we
 * cannot get the required input, we return BATTERY_UNKNOWN.
 *
 * @return the battery status
 */
enum charge_status_t battery_get_status(void);

/**
 * Calculate the next action of the battery charging state machine
 *
 * We use the state as provided to decide what to do next.
 *
 * @param state		Battery state, from battery_get_state()
 * @param config	Charging configuration
 * @return Requested battery action, BATTERY_ACT_...
 */
enum battery_action_t battery_calculate_action(
		struct battery_charge_state *state,
		const struct battery_charge_config *config);

/**
 * Gets the current charge state of the battery system.
 *
 * @param state		Place to put battery state
 *
 * @return 0 if ok, -1 on error
 */
int battery_get_charge_state(struct battery_charge_state *state);

/**
 * Get ready to start the battery state machine
 *
 * @param state		Returns resulting battery state
 */
int battery_charge_init(struct battery_charge_state *state);

/**
 * Enable / disable the battery charger
 *
 * @param enable	0 to disable charging, non-zero to enable
 */
int battery_set_charge_enable(int enable);

/**
 * Set up the battery system
 *
 * This communicates with the battery to make sure it is present and ready
 *
 * @return 0 if ok, -1 if battery missing
 */
int battery_init(void);

#endif
