/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <mmc.h>
#include <part.h>
#include <usb.h>
#include <chromeos/common.h>
#include <linux/list.h>

/* Import the header files from vboot_reference. */
#include <vboot_api.h>

#define PREFIX			"boot_device: "

/*
 * Total number of storage devices, USB + MMC + reserved.
 * MMC is 2 now. Reserve extra 3 for margin of safety.
 */
#define MAX_DISK_INFO		(USB_MAX_STOR_DEV + 2 + 3)

/* TODO Move these definitions to vboot_wrapper.h or somewhere like that. */
enum {
	VBERROR_DISK_NO_DEVICE = 1,
	VBERROR_DISK_OUT_OF_RANGE,
	VBERROR_DISK_READ_ERROR,
	VBERROR_DISK_WRITE_ERROR,
};

static uint32_t get_dev_flags(const block_dev_desc_t *dev)
{
	return dev->removable ? VB_DISK_FLAG_REMOVABLE : VB_DISK_FLAG_FIXED;
}

static const char *get_dev_name(const block_dev_desc_t *dev)
{
	/*
	 * See the definitions of IF_TYPE_* in /include/part.h.
	 * 8 is max size of strings, the "UNKNOWN".
	 */
	static const char all_disk_types[IF_TYPE_MAX][8] = {
		"UNKNOWN", "IDE", "SCSI", "ATAPI", "USB",
		"DOC", "MMC", "SD", "SATA"};

	if (dev->if_type >= IF_TYPE_MAX) {
		return all_disk_types[0];
	}
	return all_disk_types[dev->if_type];
}

static void init_usb_storage(void)
{
	/*
	 * We should stop all USB devices first. Otherwise we can't detect any
	 * new devices.
	 */
	usb_stop();

	if (usb_init() >= 0)
		usb_stor_scan(/*mode=*/1);
}

block_dev_desc_t *iterate_mmc_device(int *index_ptr)
{
	struct mmc *mmc;
	int index;

	for (index = *index_ptr; (mmc = find_mmc_device(index)); index++) {
		/* Skip device that cannot be initialized */
		if (!mmc_init(mmc))
			break;
	}

	/*
	 * find_mmc_device() returns a pointer from a global linked list. So
	 * &mmc->block_dev is not a dangling pointer after this function
	 * is returned.
	 */

	*index_ptr = index + 1;
	return mmc ? &mmc->block_dev : NULL;
}

block_dev_desc_t *iterate_usb_device(int *index_ptr)
{
	return usb_stor_get_dev((*index_ptr)++);
}

/*
 * This appends [dev] to [info] and increments [count_ptr] if [disk_flags] is a
 * subset of the flags of [dev].
 */
void add_device_if_flags_match(block_dev_desc_t *dev, const uint32_t disk_flags,
		VbDiskInfo *info, uint32_t *count_ptr)
{
	uint32_t flags = get_dev_flags(dev);

	/*
	 * Only add this storage device if the properties of disk_flags is a
	 * subset of the properties of flags.
	 */
	if ((flags & disk_flags) != disk_flags)
		return;

	info->handle = (VbExDiskHandle_t)dev;
	info->bytes_per_lba = dev->blksz;
	info->lba_count = dev->lba;
	info->flags = flags;
	info->name = get_dev_name(dev);
	(*count_ptr)++;
}

VbError_t VbExDiskGetInfo(VbDiskInfo** infos_ptr, uint32_t* count_ptr,
                          uint32_t disk_flags)
{
	VbDiskInfo *infos;
	uint32_t count, max_count;
	int iterator_state;
	block_dev_desc_t *dev;

	*infos_ptr = NULL;
	*count_ptr = 0;

	/* We return as many disk infos as possible. */
	/*
	 * We assume there is only one non-removable storage device. So if the
	 * caller asks for non-removable devices, we return at most one device.
	 * Otherwise we return at most MAX_DISK_INFO device.
	 */
	max_count = (disk_flags & VB_DISK_FLAG_REMOVABLE) ? MAX_DISK_INFO : 1;

	/* count is the number of device returned */
	infos = (VbDiskInfo *)VbExMalloc(sizeof(VbDiskInfo) * max_count);
	count = 0;

	iterator_state = 0;
	while (count < max_count && (dev = iterate_mmc_device(&iterator_state)))
		add_device_if_flags_match(dev, disk_flags,
				infos + count, &count);

	/* Skip probing USB device if not require removable devices. */
        if (count < max_count && (disk_flags & VB_DISK_FLAG_REMOVABLE)) {
		/* Initialize USB device before probing them. */
		init_usb_storage();

		iterator_state = 0;
		while (count < max_count &&
				(dev = iterate_usb_device(&iterator_state)))
			add_device_if_flags_match(dev, disk_flags,
					infos + count, &count);
	}

	if (count) {
		*infos_ptr = infos;
		*count_ptr = count;
	} else
		VbExFree(infos);

	return VBERROR_SUCCESS;
}

VbError_t VbExDiskFreeInfo(VbDiskInfo *infos, VbExDiskHandle_t preserve_handle)
{
	/* We do nothing for preserve_handle as we keep all the devices on. */
	VbExFree(infos);
	return VBERROR_SUCCESS;
}

VbError_t VbExDiskRead(VbExDiskHandle_t handle, uint64_t lba_start,
                       uint64_t lba_count, void *buffer)
{
	block_dev_desc_t *dev = (block_dev_desc_t *)handle;

	if (dev == NULL)
		return VBERROR_DISK_NO_DEVICE;

	if (lba_start >= dev->lba || lba_start + lba_count > dev->lba)
		return VBERROR_DISK_OUT_OF_RANGE;

	if (dev->block_read(dev->dev, lba_start, lba_count, buffer)
			!= lba_count)
		return VBERROR_DISK_READ_ERROR;

	return VBERROR_SUCCESS;
}

VbError_t VbExDiskWrite(VbExDiskHandle_t handle, uint64_t lba_start,
                        uint64_t lba_count, const void *buffer)
{
	block_dev_desc_t *dev = (block_dev_desc_t *)handle;

	/* TODO Replace the error codes with pre-defined macro. */
	if (dev == NULL)
		return VBERROR_DISK_NO_DEVICE;

	if (lba_start >= dev->lba || lba_start + lba_count > dev->lba)
		return VBERROR_DISK_OUT_OF_RANGE;

	if (dev->block_write(dev->dev, lba_start, lba_count, buffer)
			!= lba_count)
		return VBERROR_DISK_WRITE_ERROR;

	return VBERROR_SUCCESS;
}
