/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_daisy_h__
#define __configs_chromeos_daisy_h__

#include <configs/smdk5250.h>

#define CONFIG_INITRD_ADDRESS 0x42000000

#include <configs/chromeos.h>

/*
 * Extra bootargs used for direct booting, but not for vboot.
 * - console of the board
 * - debug and earlyprintk: easier to debug; they could be removed later
 */
#define CONFIG_DIRECT_BOOTARGS \
	"console=ttySAC3,115200 debug earlyprintk"

/* TODO(clchiou): Provide actual value to it later */
/* Standard input, output and error device of U-Boot console. */
#define CONFIG_STD_DEVICES_SETTINGS ""

/* TODO(clchiou): Disable them just for now; re-enable later */
#undef CONFIG_CHROMEOS
#undef CONFIG_OF_LOAD_ENVIRONMENT
#undef CONFIG_OF_UPDATE_FDT_BEFORE_BOOT

/*
 * TODO(clchiou): We override mmc_setup here because today on exynos U-Boot and
 * kernel disagree on eMMC and SD card index.  We should fix this so that the
 * index is consistent: 0 for internal (eMMC) and 1 for external (SD card).
 *
 * I will remove overriding mmc_setup after this is fixed.
 */
/* Replace default CONFIG_EXTRA_ENV_SETTINGS */
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS \
	\
	"mmc_setup=" \
		"mmc rescan ${devnum}; " \
		"setenv devtype mmc; " \
		"if test ${devnum} -eq 0; then " \
			"setenv kernel_devnum 1; " \
		"else " \
			"setenv kernel_devnum 0; " \
		"fi; " \
		"setenv devname mmcblk${kernel_devnum}p\0"

/* Replace default CONFIG_BOOTCOMMAND */
#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND CONFIG_NON_VERIFIED_BOOTCOMMAND

#endif /* __configs_chromeos_daisy_h__ */
