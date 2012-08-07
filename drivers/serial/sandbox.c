/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * This provide a test serial port. It provides an emulated serial port where
 * a test program and read out the serial output and inject serial input for
 * U-Boot.
 */

#include <common.h>
#include <os.h>

/*
 *
 *   serial_buf: A buffer that holds keyboard characters for the
 *		 Sandbox U-boot.
 *
 * invariants:
 *   serial_buf_write		 == serial_buf_read -> empty buffer
 *   (serial_buf_write + 1) % 16 == serial_buf_read -> full buffer
 */
static char serial_buf[16];
static unsigned int serial_buf_write;
static unsigned int serial_buf_read;

int serial_init(void)
{
	os_tty_raw(0);
	return 0;
}

void serial_setbrg(void)
{
}

void serial_putc(const char ch)
{
	os_write(1, &ch, 1);
}

void serial_puts(const char *str)
{
	os_write(1, str, strlen(str));
}

static unsigned int increment_buffer_index(unsigned int index)
{
	return (index + 1) % ARRAY_SIZE(serial_buf);
}

int serial_getc(void)
{
	int result = serial_buf[serial_buf_read];

	if (serial_buf_read == serial_buf_write)
		return 0;	/* buffer empty */

	serial_buf_read = increment_buffer_index(serial_buf_read);
	return result;
}

int serial_tstc(void)
{
	const unsigned int next_index =
		increment_buffer_index(serial_buf_write);
	ssize_t count;

	if (next_index == serial_buf_read)
		return 1;	/* buffer full */

	count = os_read_no_block(0, &serial_buf[serial_buf_write], 1);
	if (count == 1)
		serial_buf_write = next_index;
	return serial_buf_write != serial_buf_read;
}
