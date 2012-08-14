/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>

#include "input.h"
#include "stdio_dev.h"

#include "asm/sandbox-api.h"

/**
 * Read a key
 *
 * TODO: U-Boot wants 0 for no key, but Ctrl-@ is a valid key...
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
static int sandbox_keyboard_getc(void)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->device_id = SB_KEYBOARD;
	dbc->command_data[0] = 1; /* getc */
	sandbox_ring_doorbell();
	return dbc->dbc_buf[0];
}

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
static int sandbox_keyboard_tstc(void)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->device_id = SB_KEYBOARD;
	dbc->command_data[0] = 0; /* tstc */
	sandbox_ring_doorbell();
	return dbc->dbc_buf[0];
}

/**
 * Set up the keyboard.
 *
 * @return 0 if ok, negative on error
 */
static int sandbox_keyboard_initialize(void)
{
	return 0;
}


int sandbox_keyboard_init(void)
{
	struct stdio_dev dev;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "sandbox-keyb");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = sandbox_keyboard_getc;
	dev.tstc = sandbox_keyboard_tstc;
	dev.start = sandbox_keyboard_initialize;
	return input_stdio_register(&dev);
}
