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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"
#include "doorbell-command.h"
#include "asm/sandbox-api.h"
#include "sd_mmc.h"

char   *mmc_file[2];		/* file implementing MMC device. */

static unsigned mmc_blocklen = 512;   /* block size */

/*
 * The method to erase a block is implemented through three commands;
 * the first two set the start & end block of the erase and the third
 * executes the erase.
 */
static unsigned mmc_erase_group_start_block;
static unsigned mmc_erase_group_end_block;

/*
 * Extended CSD register: extracted from Daisy MMC.
 *
 * TODO(thutt@chromium.org): allow this data to be loaded from a file
 * so that it can be replaced with different data.
 */
static const unsigned char ext_csd_register[512] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x07, 0x01, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1f,
	0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x00, 0x02, 0x00, 0x07, 0x00, 0x02, 0x01,
	0x22, 0x22, 0x22, 0x22, 0x00, 0x0a, 0x0a, 0x0a,
	0x0a, 0x0a, 0x0a, 0x00, 0x00, 0xa0, 0xd5, 0x01,
	0x00, 0x11, 0x00, 0x07, 0x08, 0x08, 0x01, 0x01,
	0x08, 0x07, 0x10, 0x00, 0x07, 0x96, 0x96, 0x15,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22,
	0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64,
	0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned get_mmc_device(const struct doorbell_command_t *dbc)
{
	return dbc->device_id - SB_MMC0; /* 0-based device numbers */
}

int mmc_validate_arguments(void)
{
	unsigned error = 0;

	/* No file means that there is no MMC device; files assigned
	 * sequentially, no need to check more than first element.
	 */
	if (mmc_file[0] == NULL)
		return 0;

	/*
	 * TODO(thutt@chromium.org): When CSD register can be loaded,
	 * validate it.
	 */

	return error;
}

void mmc_initialize(struct doorbell_t *db)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(db->mmc); ++i)
		db->mmc[i].mmc_enabled = mmc_file[i] != NULL;
}

static int open_mmc_file(unsigned device)
{
	if (mmc_file[device] == NULL)
		return -1;

	return open(mmc_file[device], O_RDWR | O_CREAT, 0600);
}

static __u64 mmc_size(unsigned device)
{
	const struct doorbell_t *db = sandbox_get_doorbell();
	if (device >= ARRAY_SIZE(db->mmc))
		return 0;
	return db->mmc[device].mmc_capacity;
}

/**
 * Performs a seek to a point in the MMC backing file.
 * @param: dbc	Pointer to doorbell command area.
 * @param: fd	file descriptor of backing file.
 * @param: block_start	starting block for the seek
 * @param: n_blocks	number of blocks in operational region
 * @return: result >=  0 -> success; seeked to position.
 *          result == -1 -> failure; lseek failed, errno set.
 *          result == -2 -> failure; seek out-of-bounds for MMC device;
 *					errno not set.
 */
static off_t mmc_bounded_seek(struct doorbell_command_t *dbc,
			      int fd,
			      unsigned block_start,
			      unsigned n_blocks)
{
	const __u64 offset = block_start * (__u64)mmc_blocklen;
	const __u64 len = n_blocks * (__u64)mmc_blocklen;

	if (offset + len > mmc_size(get_mmc_device(dbc))) {
		verbose("%s: bounds failure [%u..%u)  size: %u\n",
			offset, offset + len, mmc_size(get_mmc_device(dbc)));
		command_failure(dbc, dbc->device_id);
		return (off_t)-2; /* out-of-bounds */
	}

	return lseek(fd, offset, SEEK_SET);
}

static void mmc_erase_group_start(struct doorbell_command_t *dbc,
				  unsigned start)
{
	mmc_erase_group_start_block = start;
}

static void mmc_erase_group_end(struct doorbell_command_t *dbc, unsigned end)
{
	mmc_erase_group_end_block = end;
}

static void mmc_set_blocklen(struct doorbell_command_t *dbc, unsigned len)
{
	mmc_blocklen = len;
}

static void mmc_write_block(struct doorbell_command_t *dbc,
			    int fd,
			    unsigned start,
			    unsigned length,
			    void *buf)
{
	const off_t s = mmc_bounded_seek(dbc, fd, start, length);

	if (s <= (off_t)-1) {
		command_failure(dbc, dbc->device_id);
	} else {
		/*
		 *  TODO(thutt@chromium.org):
		 *  Ignore possible 32-bit overflow in calculating the
		 *  length of the write.  Practically, you can't write
		 *  that much data because the sandbox has much less
		 *  memory.
		 */
		const unsigned len = length * mmc_blocklen;
		const ssize_t s = write(fd, buf, len);

		if (s < (ssize_t)len) {
			/*
			 * Error writing file, or a full write was not
			 * completed.  Fail the command.
			 */
			command_failure(dbc, dbc->device_id);
		}
	}
}

/**
 * Erase a block range on the MMC
 * @param: dbc	Pointer to doorbell command area.
 * @param: fd	file descriptor of backing file.
 * @param: start	start block number
 * @param: end	end block number
 */
static void mmc_erase_group(struct doorbell_command_t *dbc, int fd,
			    unsigned start, unsigned end)
{
	unsigned char  *buf;
	const off_t s = mmc_bounded_seek(dbc, fd, start, end + 1 - start);

	if (s <= (off_t)-1) {
		command_failure(dbc, dbc->device_id);
		return;
	}

	buf = malloc(mmc_blocklen);
	if (buf == NULL) {
		command_failure(dbc, dbc->device_id);
		return;
	}

	memset(buf, '\0', sizeof(buf));

	while (dbc->result == 0 && start <= end) {
		mmc_write_block(dbc, fd, start, 1, buf);
		++start;
	}
	free(buf);
}

static void mmc_read_block(struct doorbell_command_t *dbc,
			   int fd,
			   unsigned start,
			   unsigned length,
			   void *buf)
{
	const unsigned len = length * mmc_blocklen;
	const off_t s = mmc_bounded_seek(dbc, fd, start, length);

	if (s == (off_t)-2) {
		/* Out-of-bounds range */
		command_failure(dbc, dbc->device_id);
	} else if (s == (off_t)-1) {
		/* Unable to seek. Return all-bits-set. */
		memset(buf, 0xff, len);
	} else {
		/*
		 * TODO(thutt@chromium.org):
		 *  Ignore possible 32-bit overflow in calculating the
		 *  length of the read.  Practically, you can't read
		 *  that much data because the sandbox has much less
		 *  memory.
		 */
		const ssize_t s = read(fd, buf, len);

		if (s == -1) {
			command_failure(dbc, dbc->device_id);
		} else if (s < (ssize_t)len) {
			/*
			 * A partial read has occurred.
			 *
			 * It is an in-bounds read of the physical
			 * device because we passed the bounds check
			 * above.
			 *
			 * 'len' bytes were requested to be read, but
			 * the file associated with the MMC has not
			 * been written with enough data yet.  We
			 * treat the remaining bytes as uninitialized
			 * and set them to 0xff.
			 */
			memset(buf + s, 0xff, (ssize_t)len - s);
		}
	}
}

static void mmc_unknown_command(struct doorbell_command_t *dbc,
				unsigned command)
{
	fprintf(stderr, "Unhandled MMC command '%u'\n", command);
	command_failure(dbc, dbc->device_id);
}

static void mmc_clear_results(struct doorbell_command_t *dbc)
{
	dbc->command_data[8] = 0;
	dbc->command_data[9] = 0;
	dbc->command_data[10] = 0;
	dbc->command_data[11] = 0;
}

static void mmc_send_op_cond(struct doorbell_command_t *dbc)
{
	const int ocr_busy = 0x80000000;
	const int ocr_voltage_mask = 0x007FFF80;
	const int ocr_access_mode = 0x60000000;

	dbc->command_data[8] = (ocr_busy |
				ocr_voltage_mask |
				ocr_access_mode);
	dbc->command_data[9]  = 0;
	dbc->command_data[10] = 0;
	dbc->command_data[11] = 0;
}

static void mmc_send_cid_register(struct doorbell_command_t *dbc)
{
	/* Dummy MMC CID values */
	dbc->command_data[8] = 0x45010053;
	dbc->command_data[9] = 0x454d3136;
	dbc->command_data[10] = 0x479071eb;
	dbc->command_data[11] = 0xf3eb3fef;
}

static void mmc_send_ext_csd_register(struct doorbell_command_t *dbc)
{
	void *buf = (void *)(uintptr_t)dbc->command_data[4];

	memcpy(buf, ext_csd_register, sizeof(ext_csd_register));
}

static void mmc_send_csd_register(struct doorbell_command_t *dbc)
{
	/* Dummy MMC CSD values */
	dbc->command_data[8]  = 0xd00f0032;
	dbc->command_data[9]  = 0xf5903ff;
	dbc->command_data[10] = 0xffffffff;
	dbc->command_data[11] = 0x92404011;
}

void mmc_command(struct doorbell_command_t *dbc)
{
	/* See sandbox_mmc.c for the command_data[] layout. */
	int fd;
	unsigned command = dbc->command_data[0];

	fd = open_mmc_file(get_mmc_device(dbc));

	if (fd == -1) {
		fprintf(stderr, "Unable to open/create '%s'\n",
			mmc_file[get_mmc_device(dbc)]);
		command_failure(dbc, dbc->device_id);
		return;
	}

	mmc_clear_results(dbc);

	switch (command) {
	case MMC_CMD_GO_IDLE_STATE:
		/* NOP */
		break;
	case MMC_CMD_SEND_OP_COND:
		mmc_send_op_cond(dbc);
		break;
	case MMC_CMD_ALL_SEND_CID:
		mmc_send_cid_register(dbc);
		break;
	case MMC_CMD_SET_RELATIVE_ADDR: /* SD_CMD_SEND_RELATIVE_ADDR */
		dbc->command_data[8]  = 0x500;
		break;
	case MMC_CMD_SWITCH:
		/* NOP */
		break;
	case MMC_CMD_SELECT_CARD:
		dbc->command_data[8] = MMC_STATUS_RDY_FOR_DATA | MMC_STATUS;
		break;
	case MMC_CMD_SEND_EXT_CSD:
		if (dbc->command_data[4] == 0)	/* SD_CMD_SEND_IF_COND */
			command_timeout(dbc, dbc->device_id);
		else				/* MMC_CMD_SEND_EXT_CSD */
			mmc_send_ext_csd_register(dbc);
		break;
	case MMC_CMD_SEND_CSD:
		mmc_send_csd_register(dbc);
		break;
	case MMC_CMD_STOP_TRANSMISSION:
		verbose("%s: ignored '%d' command\n", __func__, command);
		break;
	case MMC_CMD_SEND_STATUS:
		dbc->command_data[8] = MMC_STATUS_RDY_FOR_DATA | MMC_STATUS;
		break;
	case MMC_CMD_SET_BLOCKLEN:
		mmc_set_blocklen(dbc, dbc->command_data[2]);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK: {
		const unsigned start = dbc->command_data[2];
		void *buf = (void *)(uintptr_t)dbc->command_data[4];
		mmc_read_block(dbc, fd, start, 1, buf);
		break;
	}
	case MMC_CMD_READ_MULTIPLE_BLOCK: {
		const unsigned start = dbc->command_data[2];
		void *buf = (void *)(uintptr_t)dbc->command_data[4];
		const unsigned len = dbc->command_data[6];

		mmc_read_block(dbc, fd, start, len, buf);
		break;
	}
	case MMC_CMD_WRITE_SINGLE_BLOCK: {
		const unsigned start = dbc->command_data[2];
		void *buf = (void *)(uintptr_t)dbc->command_data[4];

		mmc_write_block(dbc, fd, start, 1, buf);
		break;
	}
	case MMC_CMD_WRITE_MULTIPLE_BLOCK: {
		const unsigned start = dbc->command_data[2];
		void *buf = (void *)(uintptr_t)dbc->command_data[4];
		const unsigned len = dbc->command_data[6];

		mmc_write_block(dbc, fd, start, len, buf);
		break;
	}
	case MMC_CMD_ERASE_GROUP_START:
		mmc_erase_group_start(dbc, dbc->command_data[2]);
		break;
	case MMC_CMD_ERASE_GROUP_END:
		mmc_erase_group_end(dbc, dbc->command_data[2]);
		break;
	case MMC_CMD_ERASE:
		mmc_erase_group(dbc, fd,
				mmc_erase_group_start_block,
				mmc_erase_group_end_block);
		break;
	case SD_CMD_APP_SEND_OP_COND:
		dbc->command_data[8] = 0;
		command_timeout(dbc, dbc->device_id);
		break;
	case SD_CMD_APP_SEND_SCR:
		/* Do not support higher clock speeds */
		command_failure(dbc, dbc->device_id);
		break;
	case MMC_CMD_APP_CMD:
		dbc->command_data[8] = OCR_BUSY | OCR_HCS;
		break;
	default:
		mmc_unknown_command(dbc, command);
		break;
	}
	close(fd);
}
