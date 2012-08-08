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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "lib.h"
#include "asm/sandbox-api.h"
#include "doorbell-command.h"

static char const * const device_names[SB_N_DEVICES] = {
#define D(_id, _str) _str,
		DEVICES
#undef D
};

static void command_failure_with_error(struct doorbell_command_t *dbc,
				       unsigned device,
				       int error)
{
	verbose("%s: device: %s; command failed\n", __func__,
		device_names[device]);
	dbc->result = error;
}

void command_failure(struct doorbell_command_t *dbc, unsigned device)
{
	command_failure_with_error(dbc, device, ~0);
}

void command_timeout(struct doorbell_command_t *dbc, unsigned device)
{
	command_failure_with_error(dbc, device, -19);
}
