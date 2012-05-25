/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef __SMARTBAT_H_
#define __SMARTBAT_H_

/* Values for the status register */
enum {
	SMARTBAT_ST_DISCHARGED	= 1 << 4,
	SMARTBAT_ST_CHARGED	= 1 << 5,
	SMARTBAT_ST_DISCHARGING	= 1 << 6,
	SMARTBAT_ST_INITED	= 1 << 7,

	SMARTBAT_ST_TIME_LOW	= 1 << 8,
	SMARTBAT_ST_CAPACITY_LOW	= 1 << 9,
	SMARTBAT_ST_STOP_DISCHARGE = 1 << 11,
	SMARTBAT_ST_OVER_TEMP	= 1 << 12,
	SMARTBAT_ST_STOP_CHARGE	= 1 << 14,
	SMARTBAT_ST_OVER_CHARGE	= 1 << 15,
};

struct battery_info;

int smartbat_get_info(struct battery_info *info);

/**
 * Read the battery temperature
 *
 * @param milli_degrees_c	Returns battery termperature in units of
				milli-degrees celsius
 * @return 0 if ok, -1 on error
 */
int smartbat_get_temperature(int *milli_degrees_c);

/**
 * Read the battery status
 *
 * @return battery status (see SMARTBAT_ST_... for flags) or -1 on error
 */
int smartbat_get_status(void);

/**
 * Initialize the SMARTBAT battery.
 *
 * @return	0 on success, non-0 on failure
 */
int smartbat_init(void);

#endif /* __SMARTBAT_H_ */
