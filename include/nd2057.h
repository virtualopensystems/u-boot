/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef __ND2057_H_
#define __ND2057_H_

/* Values for the status register */
enum {
	ND2057_ST_DISCHARGED	= 1 << 4,
	ND2057_ST_CHARGED	= 1 << 5,
	ND2057_ST_DISCHARGING	= 1 << 6,
	ND2057_ST_INITED	= 1 << 7,

	ND2057_ST_TIME_LOW	= 1 << 8,
	ND2057_ST_CAPACITY_LOW	= 1 << 9,
	ND2057_ST_STOP_DISCHARGE = 1 << 11,
	ND2057_ST_OVER_TEMP	= 1 << 12,
	ND2057_ST_STOP_CHARGE	= 1 << 14,
	ND2057_ST_OVER_CHARGE	= 1 << 15,
};

struct battery_info;

int nd2057_get_info(struct battery_info *info);

/**
 * Read the battery temperature
 *
 * @param milli_degrees_c	Returns battery termperature in units of
				milli-degrees celsius
 * @return 0 if ok, -1 on error
 */
int nd2057_get_temperature(int *milli_degrees_c);

/**
 * Read the battery status
 *
 * @return battery status (see ND2057_ST_... for flags) or -1 on error
 */
int nd2057_get_status(void);

/**
 * Initialize the ND2057 battery.
 *
 * @return	0 on success, non-0 on failure
 */
int nd2057_init(void);

#endif /* __ND2057_H_ */
