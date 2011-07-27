/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CHROMEOS_COMMON_H_
#define CHROMEOS_COMMON_H_

#include <vboot_api.h>

#if defined VBOOT_DEBUG
#define VBDEBUG(fmt, args...) VbExDebug(fmt ,##args)
#elif defined DEBUG
#define VBDEBUG debug
#else
#define VBDEBUG(fmt, args...)
#endif

/*
 * VBDEBUG(), which indirectly calls printf(), has an internal buffer on output
 * string, and so cannot output very long string. Thus, if you want to print a
 * very long string, please use VBDEBUG_PUTS(), which calls puts().
 */
#if defined VBOOT_DEBUG || defined DEBUG
#define VBDEBUG_PUTS(str) puts(str)
#else
#define VBDEBUG_PUTS(str)
#endif


enum {
	BOOTSTAGE_VBOOT_TWOSTOP = BOOTSTAGE_ID_USER,
	BOOTSTAGE_VBOOT_TWOSTOP_INIT,
	BOOTSTAGE_VBOOT_SELECT_AND_SET,
	BOOTSTAGE_VBOOT_TWOSTOP_MAIN_FIRMWARE,

	BOOTSTAGE_VBOOT_LAST
};

/* this function is implemented along with vboot_api */
int display_clear(void);

/* set up the vbexport library */
int vbexport_init(void);

/* put this prototype here for now */
unsigned timer_get_us(void);

/* this function is implemented along with vboot_api */
int display_clear(void);

#endif /* CHROMEOS_COMMON_H_ */
