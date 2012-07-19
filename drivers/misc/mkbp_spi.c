/*
 * Chromium OS mkbp driver - SPI interface
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
#include <mkbp.h>
#include <spi.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

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
int mkbp_spi_command(struct mkbp_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t *din, int din_len)
{
	int in_bytes = din_len + MSG_PROTO_BYTES;
	const uint8_t *p, *end;
	int len = 0;
	int rv;

	if (cmd_version != 0) {
		debug("%s: Command version >0 unsupported\n", __func__);
		return -1;
	}

	/*
	 * Sanity-check input size to make sure it plus transaction overhead
	 * fits in the internal device buffer.
	 */
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/* Clear input buffer so we don't get false hits for MSG_HEADER */
	memset(dev->din, '\0', in_bytes);

	if (spi_claim_bus(dev->spi)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}


	/* Send command */
#ifdef CONFIG_NEW_SPI_XFER
	rv = spi_xfer(dev->spi, &cmd, 8, dev->din, 8));
#else
	rv = spi_xfer(dev->spi, 8, &cmd, dev->din,
		      SPI_XFER_BEGIN | SPI_XFER_END);
#endif

	if (rv) {
		debug("%s: Cannot send command via SPI\n", __func__);
		spi_release_bus(dev->spi);
		return -1;
	}

	/* Send output data and receive input data */
#ifdef CONFIG_NEW_SPI_XFER
	rv = spi_xfer(dev->spi, dout, dout_len * 8,
		      dev->din, in_bytes * 8)) {
#else
	/*
	 * TODO(sjg): Old SPI XFER doesn't support sending params before
	 * reading response, so this will probably clock the wrong amount of
	 * data.
	 */
	rv = spi_xfer(dev->spi, max(dout_len, in_bytes) * 8,
		      dev->dout, dev->din,
		      SPI_XFER_BEGIN | SPI_XFER_END);
#endif

	spi_release_bus(dev->spi);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	/* Scan to start of reply */
	for (p = dev->din, end = p + in_bytes; p < end; p++) {
		if (*p == MSG_HEADER)
			break;
	}

	if (end - p > 2)
		len = p[1] + (p[2] << 8);
	else
		debug("%s: Message header not found\n", __func__);

	if (!len || len > end - p) {
		debug("%s: Message received is too short, len=%d, bytes=%d\n",
		      __func__, len, end - p);
		len = -1;
	}

	if (len > 0) {
		p += MSG_HEADER_BYTES;
		len -= MSG_PROTO_BYTES;	/* remove header, checksum, trailer */

		/* Response code is first byte of message */
		if (p[0] != EC_RES_SUCCESS)
			return -(int)(p[0]);

		/* Anything else is the response data */
		len = min(len - 1, din_len);
		if (len)
			memcpy(din, p + 1, len);
	}
	return len;
}
