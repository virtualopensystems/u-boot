/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_daisy_h__
#define __configs_chromeos_daisy_h__

#include <configs/smdk5250.h>

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE      exynos5250-daisy

/* Generally verified boot needs more heap space */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN	(32 << 20)

#define CONFIG_INITRD_ADDRESS 0x42000000

#include <configs/chromeos.h>

#define CONFIG_CHROMEOS_USB

/* Support vboot flag reading from GPIO hardwrae */
#define CONFIG_CHROMEOS_GPIO_FLAG

/* Support vboot flag reading from EC */
#define CONFIG_CHROMEOS_MKBP_FLAG

/* Use the default arch_phys_memset implementation */
#define CONFIG_PHYSMEM

/* TPM */
#define CONFIG_INFINEON_TPM_I2C
#define CONFIG_TPM_SLB9635_I2C
/* FIXME(crosbug.com/28214): Revisit this burst limitation */
#define CONFIG_TPM_I2C_BURST_LIMITATION		3

/* Adjust the display resolution. */
#undef MAIN_VRESOL_VAL
#undef MAIN_HRESOL_VAL
#define MAIN_VRESOL_VAL 0x300
#define MAIN_HRESOL_VAL 0x556
#undef LCD_XRES
#undef LCD_YRES
#define LCD_XRES 1366
#define LCD_YRES 768

/*
 * Extra bootargs used for direct booting, but not for vboot.
 * - console of the board
 * - debug and earlyprintk: easier to debug; they could be removed later
 */
#define CONFIG_DIRECT_BOOTARGS \
	"console=ttySAC3," STRINGIFY(CONFIG_BAUDRATE) " debug earlyprintk"

/* Standard input, output and error device of U-Boot console. */
#define CONFIG_STD_DEVICES_SETTINGS 	EXYNOS_DEVICE_SETTINGS

#define CONFIG_CHROMEOS_SD_TO_SPI \
	"sd_to_spi=echo Flashing U-Boot from SD card to SPI flash; " \
	"if mmc dev 1 && " \
	"mmc rescan && " \
	"mmc read 40008000 1 1000 && " \
	"sf probe 1:0 && " \
	"sf update 40008000 0 80000; then " \
	"echo Flash completed; else " \
	"echo Flash failed; " \
	"fi\0"

/* Replace default CONFIG_EXTRA_ENV_SETTINGS */
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS \
	CONFIG_CHROMEOS_SD_TO_SPI \
	"dev_extras=daisy\0"

/* Replace default CONFIG_BOOTCOMMAND */
#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND CONFIG_NON_VERIFIED_BOOTCOMMAND

#endif /* __configs_chromeos_daisy_h__ */
