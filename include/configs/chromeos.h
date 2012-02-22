/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_h__
#define __configs_chromeos_h__

/*
 * This config file defines platform-independent settings that a verified boot
 * firmware must have.
 */

/* Enable verified boot */
#define CONFIG_CHROMEOS

/* Enable test codes */
#ifdef VBOOT_DEBUG
#define CONFIG_CHROMEOS_TEST
#endif /* VBOOT_DEBUG */

/* Enable graphics display */
#define CONFIG_LCD_BMP_RLE8
#define CONFIG_LZMA
#define CONFIG_SPLASH_SCREEN

/*
 * Use the fdt to decide whether to load the environment early in start-up
 * (even before we decide if we're entering developer mode).
 */
#define CONFIG_OF_LOAD_ENVIRONMENT

/*
 * Enable this feature to embed crossystem data into device tree before booting
 * the kernel.
 */
/*
 * TODO If x86 firmware does not embedding crossystem data in a device tree (and
 * pass the tree to kernel) but uses ACPI or whatever instead, move this to
 * chromeos_tegra2_twostop.h.
 */
#define CONFIG_OF_UPDATE_FDT_BEFORE_BOOT

/*
 * This is the default kernel command line to a Chrome OS kernel. An ending
 * space character helps us concatenate more arguments.
 */
#ifndef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS
#endif
#define CHROMEOS_BOOTARGS "cros_secure " CONFIG_BOOTARGS " "

#endif /* __configs_chromeos_h__ */
