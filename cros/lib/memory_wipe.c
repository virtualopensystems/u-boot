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
#include <chromeos/common.h>
#include <chromeos/memory_wipe.h>

#include <vboot_api.h>

#define PREFIX		"memory_wipe: "

void memory_wipe_init(memory_wipe_t *wipe, uintptr_t start, uintptr_t end)
{
	wipe->whole.start = start;
	wipe->whole.end = end;
	wipe->excluded_count = 0;
}

int memory_wipe_exclude(memory_wipe_t *wipe, uintptr_t start, uintptr_t end)
{
	int i = wipe->excluded_count;

	if (i >= MAX_EXCLUDED_REGIONS) {
		VBDEBUG(PREFIX "the number of excluded regions reaches"
				"the maximum: %d\n", MAX_EXCLUDED_REGIONS);
		return -1;
	}

	while (i > 0 && wipe->excluded[i - 1].start > start) {
		wipe->excluded[i].start = wipe->excluded[i - 1].start;
		wipe->excluded[i].end = wipe->excluded[i - 1].end;
		i--;
	}
	wipe->excluded[i].start = start;
	wipe->excluded[i].end = end;
	wipe->excluded_count++;

	return 0;
}

static void zero_mem(uintptr_t start, uintptr_t end)
{
	if (end > start) {
		VBDEBUG(PREFIX "\t[0x%08x, 0x%08x)\n", start, end);
		memset((void *)start, '\0', (size_t)(end - start));
	}
}

void memory_wipe_execute(memory_wipe_t *wipe)
{
	uintptr_t addr = wipe->whole.start;
	int i;

	VBDEBUG(PREFIX "Wipe memory regions:\n");
	for (i = 0; i < wipe->excluded_count; i++) {
		zero_mem(addr, wipe->excluded[i].start);
		if (wipe->excluded[i].end > addr)
			addr = wipe->excluded[i].end;
	}
	zero_mem(addr, wipe->whole.end);
}
