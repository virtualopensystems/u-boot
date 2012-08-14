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

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "lib.h"
#include "sd_spi.h"
#include "sd_mmc.h"
#include "sd_keyboard.h"
#include "asm/sandbox-api.h"

/* ipcs -m: show shared memory. look for the value of SANDBOX_SHM_KEY. */
/* ipcrm -M 0xdeadcafe: remove the shared memory region */

static int shm_id;		/* shared memory id */
static void *shm_ptr;		/* pointer to shared memory */

static int get_shared_memory(void)
{
	return shmget(SANDBOX_SHM_KEY, SANDBOX_SHM_SIZE, S_IRWXU);
}

static int create_shared_memory(void)
{
	return shmget(SANDBOX_SHM_KEY, SANDBOX_SHM_SIZE, SANDBOX_SHM_FLAGS);
}

void remove_shared_memory(void)
{
	int id = get_shared_memory();
	if (id != -1) {
		verbose("%s: removing memory id '%d'\n", __func__, id);
		shmctl(id, IPC_RMID, NULL);
	} else
		verbose("%s: no memory found to remove\n", __func__, id);
}

void initialize_shared_memory(void)
{
	remove_shared_memory();
	shm_id = create_shared_memory();
	if (shm_id == -1) {
		perror("shmget");
		fatal("Unable to allocate shared memory");
	}

	verbose("%s: obtained shared memory: id '%d'\n", __func__, shm_id);
	shm_ptr = shmat(shm_id, SANDBOX_SHM_ADDRESS, S_IRWXU);
	if (shm_ptr == (void *)-1) {
		perror("shmat");
		remove_shared_memory();
		fatal("Unable to attach to shared memory");
	}
}

void initialize_shared_devices(void)
{
	initialize_spi(sandbox_get_doorbell());
	mmc_initialize(sandbox_get_doorbell());
	keyboard_initialize(sandbox_get_doorbell());
}
