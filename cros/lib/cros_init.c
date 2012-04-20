/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <cros/common.h>
#include <cros/boot_device.h>

int cros_init(void)
{
	int err = boot_device_init();

	if (err) {
		printf("vbexport_init: boot devices probe failed\n");
		return -1;
	}
	return 0;
}
