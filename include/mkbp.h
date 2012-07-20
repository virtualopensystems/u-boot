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
#include <fdtdec.h>
#include <mkbp_message.h>

/* Which interface is the device on? */
enum mkbp_interface_t {
	MKBPIF_NONE,
	MKBPIF_SPI,
	MKBPIF_I2C,
	MKBPIF_LPC,	/* Intel Low Pin Count interface */
};

/* Our configuration information */
struct mkbp_dev {
	enum mkbp_interface_t interface;
	struct spi_slave *spi;		/* Our SPI slave, if using SPI */
	int node;                       /* Our node */
	int parent_node;		/* Our parent node (interface) */
	unsigned int cs;		/* Our chip select */
	unsigned int addr;		/* Device address (for I2C) */
	unsigned int bus_num;		/* Bus number (for I2C) */
	unsigned int max_frequency;	/* Maximum interface frequency */
	struct fdt_gpio_state ec_int;	/* GPIO used as EC interrupt line */
	int cmd_version_is_supported;   /* Device supports command versions */

	/*
	 * These two buffers will always be dword-aligned and include enough
	 * space for up to 7 word-alignment bytes also, so we can ensure that
	 * the body of the message is always dword-aligned (64-bit).
	 *
	 * We use this alignment to keep ARM and x86 happy. Probably word
	 * alignment would be OK, there might be a small performance advantage
	 * to using dword.
	 */
	uint8_t din[ALIGN(MSG_BYTES + sizeof(int64_t), sizeof(int64_t))]
		__attribute__((aligned(sizeof(int64_t))));
	uint8_t dout[ALIGN(MSG_BYTES + sizeof(int64_t), sizeof(int64_t))]
		__attribute__((aligned(sizeof(int64_t))));
};

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
 * Get/set flash protection
 *
 * @param dev		MKBP device
 * @param set_mask	Mask of flags to set; if 0, just retrieves existing
 *                      protection state without changing it.
 * @param set_flags	New flag values; only bits in set_mask are applied;
 *                      ignored if set_mask=0.
 * @param prot          Destination for updated protection state from EC.
 * @return 0 if ok, <0 on error
 */
int mkbp_flash_protect(struct mkbp_dev *dev,
		       uint32_t set_mask, uint32_t set_flags,
		       struct ec_response_flash_protect *resp);


/**
 * Run internal tests on the mkbp interface.
 *
 * @param dev		MKBP device
 * @return 0 if ok, <0 if the test failed
 */
int mkbp_test(struct mkbp_dev *dev);

/**
 * Update the EC RW copy.
 *
 * @param dev		MKBP device
 * @param image		the content to write
 * @param imafge_size	content length
 * @return 0 if ok, <0 if the test failed
 */
int mkbp_flash_update_rw(struct mkbp_dev *dev,
			 const uint8_t  *image, int image_size);

/**
 * Return a pointer to the board's MKBP device
 *
 * This should be implemented by board files.
 *
 * @return pointer to MKBP device, or NULL if none is available
 */
struct mkbp_dev *board_get_mkbp_dev(void);


/* Internal interfaces */
int mkbp_i2c_init(struct mkbp_dev *dev, const void *blob);
int mkbp_spi_init(struct mkbp_dev *dev, const void *blob);
int mkbp_lpc_init(struct mkbp_dev *dev, const void *blob);

/**
 * Read information from the fdt for the i2c mkbp interface
 *
 * @param dev		MKBP device
 * @param blob		Device tree blob
 * @return 0 if ok, -1 if we failed to read all required information
 */
int mkbp_i2c_decode_fdt(struct mkbp_dev *dev, const void *blob);

/**
 * Send a command to an I2C MKBP device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		MKBP device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param dinp          Returns pointer to response data
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
int mkbp_i2c_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len);

int mkbp_lpc_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t *din, int din_len);
int mkbp_spi_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t *din, int din_len);

/**
 * Dump a block of data for a command.
 *
 * @param name	Name for data (e.g. 'in', 'out')
 * @param cmd	Command number associated with data, or -1 for none
 * @param data	Data block to dump
 * @param len	Length of data block to dump
 */
void mkbp_dump_data(const char *name, int cmd, const uint8_t *data, int len);
#endif
