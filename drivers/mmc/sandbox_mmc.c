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

static struct mmc mmc_dev[2];		/* MMC device. */
static struct mmc_host mmc_host;	/* MMC host device */

static void copy_id(char *dest, size_t dest_len, const char *src)
{
	strncpy(dest, src, dest_len);
	dest[dest_len - 1] = '\0';
}

/**
 * Returns the device number of 'dev'.
 *
 * @param dev	Pointer into array 'mmc_dev'
 * Result == 0 -> &mmc_dev[0]
 * Result == 1 -> &mmc_dev[1]
 */
static unsigned get_device(const struct mmc *dev)
{
	ASSERT_ON_COMPILE(ARRAY_SIZE(mmc_dev) == 2);
	return dev == &mmc_dev[1];
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
	const unsigned device = get_device(mmc);

	db->mmc[device].mmc_capacity = mmc->capacity;
	ASSERT_ON_COMPILE(ARRAY_SIZE(db->mmc) == 2);
	dbc->device_id = device == 0 ? SB_MMC0 : SB_MMC1;
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

static void register_mmc_device(struct mmc *dev)
{
	copy_id(dev->name, sizeof(dev->name), "Sandbox MMC");
	dev->priv = &mmc_host;
	dev->send_cmd = sandbox_mmc_send_cmd;
	dev->set_ios = sandbox_mmc_set_ios;
	dev->init = sandbox_mmc_core_init;
	dev->getcd = sandbox_mmc_getcd;

	/* These following values taken from the regular mmc driver. */
	dev->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	dev->host_caps = MMC_MODE_8BIT;
	dev->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;
	dev->f_min = 375000;
	dev->f_max = 48000000;
	dev->b_max = 0;
	mmc_register(dev);
}

int sandbox_mmc_init(int verbose)
{
	struct doorbell_t *db = sandbox_get_doorbell();
	unsigned i;

	ASSERT_ON_COMPILE(ARRAY_SIZE(db->mmc) ==
			  ARRAY_SIZE(mmc_dev));

	if (!db->mmc[0].mmc_enabled)	/* No MMC devices. */
		return -ENODEV;

	for (i = 0; i < ARRAY_SIZE(db->mmc); ++i) {
		if (db->mmc[i].mmc_enabled) {
			/* Only device 0 is a removable device */
			register_mmc_device(&mmc_dev[i]);
			mmc_dev[1].block_dev.removable = i == 0;
		}
	}
	return 0;
}
