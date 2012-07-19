/*
 * Chromium OS mkbp driver - I2C interface
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

/*
 * The Matrix Keyboard Protocol driver handles talking to the keyboard
 * controller chip. Mostly this is for keyboard functions, but some other
 * things have slipped in, so we provide generic services to talk to the
 * KBC.
 */

#include <common.h>
#include <i2c.h>
#include <mkbp.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

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
 * @param din           Response data (may be NULL If din_len=0)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
int mkbp_i2c_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t *din, int din_len)
{
	int old_bus = 0;
	int out_bytes = dout_len + 1;  /* cmd8, out8[dout_len] */
	int in_bytes = din_len + 2;  /* response8, in8[din_len], checksum8 */

	old_bus = i2c_get_bus_num();

	/*
	 * Sanity-check I/O sizes given transaction overhead in internal
	 * buffers.
	 */
	if (out_bytes > sizeof(dev->dout)) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/*
	 * Copy command and data into output buffer so we can do a single I2C
	 * burst transaction.
	 */
	dev->dout[0] = cmd;
	if (dout_len > 0)
		memcpy(dev->dout + 1, dout, dout_len);

	/* Set to the proper i2c bus */
	if (i2c_set_bus_num(dev->bus_num)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			dev->bus_num);
		return -1;
	}

	/* Send output data */
	if (i2c_write(dev->addr, 0, 0, dev->dout, out_bytes)) {
		debug("%s: Cannot complete I2C write to 0x%x\n",
			__func__, dev->addr);
		return -1;
	}

	/* Receive input data */
	if (i2c_read(dev->addr, 0, 0, dev->din, in_bytes)) {
		debug("%s: Cannot complete I2C read from 0x%x\n",
			__func__, dev->addr);
		return -1;
	}

	/* Return to original bus number */
	if (i2c_set_bus_num(old_bus)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			old_bus);
		return -1;
	}

	/*
	 * I2C currently uses a simpler protocol, so we don't
	 * deal with the full header; the checksum byte on the end
	 * is ignored.
	 * TODO(bhthompson): Migrate to new protocol.
	 */
	if (dev->din[0] != EC_RES_SUCCESS)
		return -(int)(dev->din[0]);

	/* Copy input data, if any */
	if (din_len)
		memcpy(din, dev->din + 1, din_len);

	return 0;
}

/**
 * Initialize I2C protocol.
 *
 * @param dev		MKBP device
 * @param blob		Device tree blob
 * @return 0 if ok, -1 on error
 */
int mkbp_i2c_init(struct mkbp_dev *dev, const void *blob)
{
	/* Decode interface-specific FDT params */
	dev->max_frequency = fdtdec_get_int(blob, dev->node,
					    "i2c-max-frequency", 100000);
	dev->bus_num = i2c_get_bus_num_fdt(blob, dev->parent_node);
	dev->addr = fdtdec_get_int(blob, dev->node, "reg", 0);

	i2c_init(dev->max_frequency, dev->addr);

	dev->cmd_version_is_supported = 0;

	return 0;
}
