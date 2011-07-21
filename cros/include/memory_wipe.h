/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/*
 * Memory wipe library for easily set and exclude memory regions that need
 * to be cleared.
 *
 * The following methods must be called in order:
 *   memory_wipe_init
 *   memory_wipe_exclude
 *   memory_wipe_execute
 */

#ifndef CHROMEOS_MEMORY_WIPE_H_
#define CHROMEOS_MEMORY_WIPE_H_

#include <linux/types.h>

#define MAX_EXCLUDED_REGIONS    16

typedef struct {
	uintptr_t start;
	uintptr_t end;
} memory_region_t;

typedef struct {
	memory_region_t whole;
	memory_region_t excluded[MAX_EXCLUDED_REGIONS];
	int excluded_count;
} memory_wipe_t;

/*
 * Initializes the memory region that needs to be cleared.
 */
void memory_wipe_init(memory_wipe_t *wipe, uintptr_t start, uintptr_t end);

/*
 * Excludes a memory region from the to-be-cleared region.
 * The function returns 0 on success; otherwise -1.
 */
int memory_wipe_exclude(memory_wipe_t *wipe, uintptr_t start, uintptr_t end);

/*
 * Executes the memory wipe to the memory regions which was not excluded.
 */
void memory_wipe_execute(memory_wipe_t *wipe);

#endif /* CHROMEOS_MEMORY_WIPE_H */
