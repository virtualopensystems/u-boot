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

/* Use the default arch_phys_memset implementation */
#define CONFIG_PHYSMEM

/* We initialize Chrome OS -specific GPIOs here */
#define CONFIG_MISC_INIT_R

/* Delay console init until after relocation (saves boot time) */
#define CONFIG_DELAY_CONSOLE

/* TPM */
#define CONFIG_INFINEON_TPM_I2C
#define CONFIG_INFINEON_TPM_I2C_BUS		3
#define CONFIG_TPM_SLB9635_I2C
#define CONFIG_TPM_I2C_BURST_LIMITATION		3

#endif /* __configs_chromeos_seaboard_h__ */
