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

uint64_t VbExGetTimer(void)
{
	uint32_t high, low;

	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return ((uint64_t)high << 32) | (uint64_t)low;
}
