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
#include <asm/arch/cpu.h>
#include <cros/power_management.h>

int is_processor_reset(void)
{
	static uint8_t inited, is_reset;
	uint32_t marker_value;

	if (!inited) {
		marker_value = exynos5_read_and_clear_spl_marker();
		is_reset = marker_value == EXYNOS5_SPL_MARKER;
		inited = 1;
	}

	return is_reset;
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
