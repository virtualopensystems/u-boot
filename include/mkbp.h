/*
 * Chromium OS mkbp driver
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _CROS_MKBP_H
#define _CROS_MKBP_H

#include <mkbp_message.h>

struct mkbp_dev;

/* Informaion returned by a key scan */
struct mbkp_keyscan {
	uint8_t data[MSG_KEYSCAN_BYTES];
};

/**
 * Read the ID of the MKBP device
 *
 * The ID is a string identifying the MKBP device.
 *
 * @param dev		MKBP device
 * @param id		Place to put the ID
 * @param maxlen	Maximum length of the ID field
 * @return 0 if ok, -1 on error
 */
int mkbp_read_id(struct mkbp_dev *dev, char *id, int maxlen);

/**
 * Read a keyboard scan from the MKBP device
 *
 * Send a message requesting a keyboard scan and return the result
 *
 * @param dev		MKBP device
 * @param scan		Place to put the scan results
 * @return 0 if ok, -1 on error
 */
int mkbp_scan_keyboard(struct mkbp_dev *dev, struct mbkp_keyscan *scan);

/**
 * Set up the Chromium OS matrix keyboard protocol
 *
 * @param blob		Device tree blob containing setup information
 * @return pointer to the mkbp device
 */
struct mkbp_dev *mkbp_init(const void *blob);

#endif
