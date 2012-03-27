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

/* Use the default arch_phys_memset implementation */
#define CONFIG_PHYSMEM

/* TPM */
#define CONFIG_INFINEON_TPM_I2C
#define CONFIG_INFINEON_TPM_I2C_BUS		3
#define CONFIG_TPM_SLB9635_I2C
/* FIXME(crosbug.com/28214): Revisit this burst limitation */
#define CONFIG_TPM_I2C_BURST_LIMITATION		3

/*
 * Extra bootargs used for direct booting, but not for vboot.
 * - console of the board
 * - debug and earlyprintk: easier to debug; they could be removed later
 */
#define CONFIG_DIRECT_BOOTARGS \
	"console=ttySAC3," STRINGIFY(CONFIG_BAUDRATE) " debug earlyprintk"

/* TODO(clchiou): Provide actual value to it later */
/* Standard input, output and error device of U-Boot console. */
#define CONFIG_STD_DEVICES_SETTINGS ""

#define CONFIG_CHROMEOS_SD_TO_SPI \
	"sd_to_spi=echo Flashing U-Boot from SD card to SPI flash; " \
	"mmc rescan; " \
	"mmc read 40008000 1 1000; " \
	"sf probe 1:0; " \
	"sf update 40008000 0 80000; " \
	"echo Flash completed\0"

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
	CONFIG_CHROMEOS_SD_TO_SPI \
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
