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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "lib.h"
#include "shared-memory.h"
#include "asm/sandbox-api.h"

void process_memory(void)
{
	while (1) {
		const struct doorbell_t *db = sandbox_get_doorbell();
		struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

		if (db->exit)
			cleanup_and_exit();

		if (dbc->doorbell) {
			dbc->result = 0;

			switch (dbc->device_id) {
			default:
				printf("Doorbell by unhandled device: %d\n",
				       dbc->device_id);
				break;
			}
			dbc->doorbell = 0;
		}
		usleep(100);	/* 0.10 seconds */
	}
}

