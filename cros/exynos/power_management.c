/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of per-board power management function */

#include <common.h>
#include <cros/power_management.h>

int is_processor_reset(void)
{
	/* TODO(chromium-os:28077) Implement is_processor_reset */
	return 0;
}

/* This function never returns */
void cold_reboot(void)
{
	/* TODO(chromium-os:28077) Implement cold_reboot */
}

/* This function never returns */
void power_off(void)
{
	/* TODO(chromium-os:28077) Implement power_off */
}
