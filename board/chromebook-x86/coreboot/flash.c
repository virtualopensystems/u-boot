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
#include <config.h>
#include <flash.h>

#ifndef CONFIG_SYS_NO_FLASH

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

unsigned long flash_init(void)
{
	printf("flash_init used but not implemented.\n");
	return 0;
}

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	printf("write_buff used but not implemented.\n");
	return 0;
}

#endif /* !CONFIG_SYS_NO_FLASH */
