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

#define CONFIG_INITRD_ADDRESS 0x44000000

#include <configs/chromeos.h>

#define CONFIG_CHROMEOS_USB

/* Support vboot flag reading from GPIO hardwrae */
#define CONFIG_CHROMEOS_GPIO_FLAG

/* Support vboot flag reading from EC */
#define CONFIG_CHROMEOS_MKBP_FLAG

/* Use the default arch_phys_memset implementation */
#define CONFIG_PHYSMEM

/* Adjust the display resolution. */
#undef MAIN_VRESOL_VAL
#undef MAIN_HRESOL_VAL
#define MAIN_VRESOL_VAL 0x300
#define MAIN_HRESOL_VAL 0x556
#undef LCD_XRES
#undef LCD_YRES
#define LCD_XRES 1366
#define LCD_YRES 768
#define CONFIG_SYS_WHITE_ON_BLACK

/*
 * Extra bootargs used for direct booting, but not for vboot.
 * - console of the board
 * - debug and earlyprintk: easier to debug; they could be removed later
 */
#define CONFIG_DIRECT_BOOTARGS \
	"console=tty1 debug clk_ignore_unused"

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
	"dtaddr=0x43000000\0"\
	"initrdaddr=0x44000000\0"\
	"boot_noinitrd=mmc dev 1 ; mmc rescan 1 ; ext2load mmc 1:3 ${loadaddr} uImage ; ext2load mmc 1:3 ${dtaddr} exynos5250-snow.dtb ; bootm ${loadaddr} - ${dtaddr}\0"\
	"boot_initrd=mmc dev 1 ; mmc rescan 1 ; ext2load mmc 1:3 ${loadaddr} uImage ; ext2load mmc 1:3 ${initrdaddr} initrd ; ext2load mmc 1:3 ${dtaddr} exynos5250-snow.dtb ; bootm ${loadaddr} ${initrdaddr} ${dtaddr}\0"\
	"bootdelay=3\0"

#ifdef CONFIG_BOOTARGS
#undef CONFIG_BOOTARGS
#endif
#define CONFIG_BOOTARGS \
	"console=tty1 root=/dev/mmcblk1p4 rw rootwait clk_ignore_unused"

/* Replace default CONFIG_BOOTCOMMAND */
#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND \
	"run boot_noinitrd"

/* Enable splash screens */
#define CONFIG_CROS_SPLASH

/* Enable simple framebuffer */
#define CONFIG_SIMPLEFB

#endif /* __configs_chromeos_daisy_h__ */
