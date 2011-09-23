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

/* Import the definition of vboot_wrapper interfaces. */
#include <vboot_api.h>

static uint64_t base_value;

uint64_t VbExGetTimer(void)
{
	uint64_t time_now;

	time_now = rdtsc();
	if (!base_value)
		base_value = time_now;

	return time_now - base_value;
}

void set_base_timer_value(uint64_t new_base)
{
	base_value = new_base;
}
