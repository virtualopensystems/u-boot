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

#include <linux/compiler.h>
#include <mkbp_message.h>

struct mkbp_dev;

/* Informaion returned by a key scan */
struct mbkp_keyscan {
	uint8_t data[MSG_KEYSCAN_BYTES];
};

/* Switch flags at mkbp_info response */
#define MKBP_SWITCH_LID_OPEN			0x01
#define MKBP_SWITCH_POWER_BUTTON_PRESSED	0x02
#define MKBP_SWITCH_WRITE_PROTECT_DISABLED	0x04
#define MKBP_SWITCH_KEYBOARD_RECOVERY		0x08
#define MKBP_SWITCH_DEDICATED_RECOVERY		0x10
#define MKBP_SWITCH_FAKE_DEVELOPER		0x20

/* Information about the matrix */
struct mbkp_info {
	uint32_t rows;
	uint32_t cols;
	uint8_t switches;
} __packed;

/* LPC command status byte masks */
/* EC has written a byte in the data register and host hasn't read it yet */
#define EC_LPC_STATUS_TO_HOST     0x01
/* Host has written a command/data byte and the EC hasn't read it yet */
#define EC_LPC_STATUS_FROM_HOST   0x02
/* EC is processing a command */
#define EC_LPC_STATUS_PROCESSING  0x04
/* Last write to EC was a command, not data */
#define EC_LPC_STATUS_LAST_CMD    0x08
/* EC is in burst mode.  Chrome EC doesn't support this, so this bit is never
 * set. */
#define EC_LPC_STATUS_BURST_MODE  0x10
/* SCI event is pending (requesting SCI query) */
#define EC_LPC_STATUS_SCI_PENDING 0x20
/* SMI event is pending (requesting SMI query) */
#define EC_LPC_STATUS_SMI_PENDING 0x40
/* (reserved) */
#define EC_LPC_STATUS_RESERVED    0x80

/* EC is busy.  This covers both the EC processing a command, and the host has
 * written a new command but the EC hasn't picked it up yet. */
#define EC_LPC_STATUS_BUSY_MASK \
	(EC_LPC_STATUS_FROM_HOST | EC_LPC_STATUS_PROCESSING)

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
 * Check if the MKBP device has an interrupt pending.
 *
 * Read the status of the external interrupt connected to the MKBP device.
 * If no external interrupt is configured, this always returns 1.
 *
 * @param dev		MKBP device
 * @return 0 if no interrupt is pending
 */
int mkbp_interrupt_pending(struct mkbp_dev *dev);

/**
 * Set up the Chromium OS matrix keyboard protocol
 *
 * @param blob		Device tree blob containing setup information
 * @return pointer to the mkbp device
 */
struct mkbp_dev *mkbp_init(const void *blob);

/**
 * Read information about the keyboard matrix
 *
 * @param dev		MKBP device
 * @param info		Place to put the info structure
 */
int mkbp_info(struct mkbp_dev *dev, struct mbkp_info *info);

/**
 * Return a pointer to the board's MKBP device
 *
 * This should be implemented by board files.
 *
 * @return pointer to MKBP device, or NULL if none is available
 */
struct mkbp_dev *board_get_mkbp_dev(void);

#endif
