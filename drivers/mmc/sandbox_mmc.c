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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <errno.h>
#include <fat.h>
#include <mmc.h>
#include <malloc.h>

#include "asm/sandbox-api.h"

struct mmc_host {
};

static struct mmc mmc_dev;		/* MMC device. */
static struct mmc_host mmc_host;	/* MMC host device */

static void copy_id(char *dest, size_t dest_len, const char *src)
{
	strncpy(dest, src, dest_len);
	dest[dest_len - 1] = '\0';
}

/**
 * Sends a command to the Sandbox daemon's MMC driver.
 *
 * @param mmc	Pointer to device
 * @param cmd	Pointer to device command
 * @param data	Pointer to device data
 * Result == 0 -> success
 * Result != 0 -> failure
 */
static int sandbox_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct doorbell_t *db = sandbox_get_doorbell();
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();
	const unsigned is_sandbox_mem = (data != NULL &&
					sandbox_in_shared_memory(data->dest));

	db->mmc.mmc_capacity = mmc->capacity;
	dbc->device_id = SB_MMC;
	dbc->command_data[0] = cmd->cmdidx;
	dbc->command_data[1] = cmd->resp_type;
	dbc->command_data[2] = cmd->cmdarg;
	dbc->command_data[3] = cmd->flags;
	dbc->command_data[4] = 0;
	dbc->command_data[5] = 0;
	dbc->command_data[6] = 0;
	dbc->command_data[7] = 0;

	if (data != NULL) {
		/*
		 * If the data buffer is not in shared memory, then
		 * it's likely a stack variable in the MMC driver.
		 * The sandbox-daemon does not have access to that
		 * memory, so use a local buffer for the transfer.
		 */
		dbc->command_data[4] = (__u32)(uintptr_t)data->dest;
		dbc->command_data[5] = data->flags;
		dbc->command_data[6] = data->blocks;
		dbc->command_data[7] = data->blocksize;
		if (!is_sandbox_mem)
			dbc->command_data[4] = (__u32)(uintptr_t)dbc->dbc_buf;
	}

	sandbox_ring_doorbell();

	if (data != NULL && !is_sandbox_mem)
		memcpy(data->dest, dbc->dbc_buf,
		       data->blocks * data->blocksize);

	cmd->response[0] = dbc->command_data[8];
	cmd->response[1] = dbc->command_data[9];
	cmd->response[2] = dbc->command_data[10];
	cmd->response[3] = dbc->command_data[11];
	return dbc->result;
}

static void sandbox_mmc_set_ios(struct mmc *mmc)
{
	/* NOP */
}

static int sandbox_mmc_core_init(struct mmc *mmc)
{
	/* NOP */
	return 0;
}


int sandbox_mmc_getcd(struct mmc *mmc)
{
	/* NOP.  Needed for linking, but not used here. */
	return 0;
}

int sandbox_mmc_init(int verbose)
{
	struct doorbell_t *db = sandbox_get_doorbell();
	struct mmc_t *mmc = &db->mmc;

	if (!mmc->mmc_enabled)	/* No MMC device. */
		return -ENODEV;

	copy_id(mmc_dev.name, sizeof(mmc_dev.name), "Sandbox MMC");
	mmc_dev.priv = &mmc_host;
	mmc_dev.send_cmd = sandbox_mmc_send_cmd;
	mmc_dev.set_ios = sandbox_mmc_set_ios;
	mmc_dev.init = sandbox_mmc_core_init;
	mmc_dev.getcd = sandbox_mmc_getcd;

	/* These following values taken from the regular mmc driver. */
	mmc_dev.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	mmc_dev.host_caps = MMC_MODE_8BIT;
	mmc_dev.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;
	mmc_dev.f_min = 375000;
	mmc_dev.f_max = 48000000;
	mmc_dev.b_max = 0;
	mmc_register(&mmc_dev);

	return 0;
}
