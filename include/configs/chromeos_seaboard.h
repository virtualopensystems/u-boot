/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_seaboard_h__
#define __configs_chromeos_seaboard_h__

/* So far all our tegra2-based boards share the seaboard config. */
#include <configs/seaboard.h>

/* We don't need to fully init the LCD - verified boot does this */
#undef BOARD_LATE_INIT

/* Support USB booting */
#define CONFIG_CHROMEOS_USB

#define CONFIG_INITRD_ADDRESS 0x12008000

#include "chromeos.h"

/* We initialize Chrome OS -specific GPIOs here */
#define CONFIG_MISC_INIT_R

/* Store the VbNvContext in the first block of the disk. */
#define CHROMEOS_VBNVCONTEXT_LBA	0

/* Delay console init until after relocation (saves boot time) */
#define CONFIG_DELAY_CONSOLE

#endif /* __configs_chromeos_seaboard_h__ */
