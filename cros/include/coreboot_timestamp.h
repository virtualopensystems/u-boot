/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

struct timestamp_entry {
	uint32_t	entry_id;
	uint64_t	entry_stamp;
} __attribute__((packed));

struct timestamp_table {
	uint64_t	base_time;
	uint32_t	max_entries;
	uint32_t	num_entries;
	struct timestamp_entry entries[0]; /* Variable number of entries */
} __attribute__((packed));

enum timestamp_id {
	TS_BEFORE_INITRAM = 1,
	TS_AFTER_INITRAM = 2,

	/* u-boot entry IDs start at 1000 */
	TS_U_BOOT_INITTED = 1000
};

void timestamp_init(void);
void timestamp_add(enum timestamp_id id, uint64_t ts_time);
void timestamp_add_now(enum timestamp_id id);

#endif
