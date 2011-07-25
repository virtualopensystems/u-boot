/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of per-board power management function */

#include <chromeos/power_management.h>
#include <common.h>

int is_processor_reset(void)
{
	printf("is_processor_reset used but not implemented.\n");
	return 1;
}

/* This function never returns */
void cold_reboot(void)
{
	printf("cold_reboot used but not implemented.\n");
}
