/*
 * Chromium OS mkbp driver - LPC interface
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
#include <command.h>
#include <mkbp.h>
#include <asm/io.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

static int wait_for_sync(struct mkbp_dev *dev)
{
	unsigned long start;

	start = get_timer(0);
	while (inb(dev->lpc_cmd) & EC_LPC_STATUS_BUSY_MASK) {
		if (get_timer(start) > 1000) {
			debug("%s: Timeout waiting for MKBP sync\n", __func__);
			return -1;
		}
	}

	return 0;
}

/**
 * Send a command to a LPC MKBP device and return the reply.
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
int mkbp_lpc_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t *din, int din_len)
{
	int ret, i;

	if (cmd_version != 0) {
		debug("%s: Command version >0 unsupported\n", __func__);
		return -1;
	}

	if (dout_len > dev->lpc_param_len) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	if (din_len > dev->lpc_param_len) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	debug_trace("cmd: %02x, ", command);
	for (i = 0; i < dout_len; i++) {
		debug_trace("%02x ", dout[i]);
		outb(dout[i], dev->lpc_param + i);
	}
	outb(cmd, dev->lpc_cmd);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	ret = inb(dev->lpc_data);
	if (ret) {
		debug("%s: MKBP result code %d\n", __func__, ret);
		return -ret;
	}

	debug_trace("resp: %02x, ", ret);
	for (i = 0; i < din_len; i++) {
		din[i] = inb(dev->lpc_param + i);
		debug_trace("%02x ", din[i]);
	}
	debug_trace("\n");

	return din_len;
}
