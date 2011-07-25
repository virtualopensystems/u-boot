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

/* Import the header files from vboot_reference. */
#include <vboot_api.h>

#define PREFIX		"nvstorage: "

VbError_t VbExNvStorageRead(uint8_t* buf)
{
	printf("VbExNvStorageRead used but not implemented.\n");
	return 1;
}

VbError_t VbExNvStorageWrite(const uint8_t* buf)
{
	printf("VbExNvStorageWrite used but not implemented.\n");
	return 1;
}
