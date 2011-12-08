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
#include <usb.h>

#include "boot_device.h"

#include <vboot_api.h>

static int is_enumerated;

static int boot_device_usb_start(uint32_t disk_flags)
{
	int enumerate = 1;

	/* If we aren't looking for removable disks, skip USB */
	if (!(disk_flags & VB_DISK_FLAG_REMOVABLE))
		return 0;

	/*
	 * if the USB devices have already been enumerated, redo it
	 * only if something has been plugged on unplugged.
	 */
	if (is_enumerated)
		enumerate = usb_detect_change();

	if (enumerate) {
		/*
		 * We should stop all USB devices first. Otherwise we can't
		 * detect any new devices.
		 */
		usb_stop();

		if (usb_init() >= 0) {
			usb_stor_scan(/*mode=*/1);
			is_enumerated = 1;
		}
	}
	return 1;
}

static int boot_device_usb_scan(block_dev_desc_t **desc, int max_devs,
			 uint32_t disk_flags)
{
	int index;

	max_devs = min(max_devs, USB_MAX_STOR_DEV);

	for (index = 0; index < max_devs; index++) {
		desc[index] = usb_stor_get_dev(index);
		if (!desc[index])
			break;
	}
	return index;
}

static struct boot_interface usb_interface = {
	.name = "usb",
	.type = IF_TYPE_USB,
	.start = boot_device_usb_start,
	.scan = boot_device_usb_scan,
};

int boot_device_usb_probe(void)
{
	return boot_device_register_interface(&usb_interface);
}
