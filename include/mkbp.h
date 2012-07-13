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
#include <ec_commands.h>
#include <mkbp_message.h>

struct mkbp_dev;

/*
 * Hard-code the number of columns we happen to know we have right now.  It
 * would be more correct to call mkbp_info() at startup and determine the
 * actual number of keyboard cols from there.
 */
#define MKBP_KEYSCAN_COLS 13

/* Information returned by a key scan */
struct mbkp_keyscan {
	uint8_t data[MKBP_KEYSCAN_COLS];
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
 * Read which image is currently running on the MKBP device.
 *
 * @param dev		MKBP device
 * @param image		Destination for image identifier
 * @return 0 if ok, <0 on error
 */
int mkbp_read_current_image(struct mkbp_dev *dev, enum ec_current_image *image);

/**
 * Read the hash of the MKBP device firmware.
 *
 * @param dev		MKBP device
 * @param hash		Destination for hash information
 * @return 0 if ok, <0 on error
 */
int mkbp_read_hash(struct mkbp_dev *dev, struct ec_response_vboot_hash *hash);

/**
 * Send a reboot command to the MKBP device.
 *
 * Note that some reboot commands (such as EC_REBOOT_COLD) also reboot the AP.
 *
 * @param dev		MKBP device
 * @param cmd		Reboot command
 * @param flags         Flags for reboot command (EC_REBOOT_FLAG_*)
 * @return 0 if ok, <0 on error
 */
int mkbp_reboot(struct mkbp_dev *dev, enum ec_reboot_cmd cmd, uint8_t flags);

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
int mkbp_info(struct mkbp_dev *dev, struct ec_response_mkbp_info *info);

/**
 * Read the host event flags
 *
 * @param dev		MKBP device
 * @param events_ptr	Destination for event flags.  Not changed on error.
 * @return 0 if ok, <0 on error
 */
int mkbp_get_host_events(struct mkbp_dev *dev, uint32_t *events_ptr);

/**
 * Clear the specified host event flags
 *
 * @param dev		MKBP device
 * @param events	Event flags to clear
 * @return 0 if ok, <0 on error
 */
int mkbp_clear_host_events(struct mkbp_dev *dev, uint32_t events);

/**
 * Return a pointer to the board's MKBP device
 *
 * This should be implemented by board files.
 *
 * @return pointer to MKBP device, or NULL if none is available
 */
struct mkbp_dev *board_get_mkbp_dev(void);

#endif
