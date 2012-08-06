/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_beaglebone_h__
#define __configs_chromeos_beaglebone_h__

#include <configs/am335x_evm.h>

#define CONFIG_ARCH_DEVICE_TREE         am335x_evm
#define CONFIG_DEFAULT_DEVICE_TREE      am335x_evm-beaglebone

/* Generally verified boot needs more heap space */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN	(32 << 20)

#define CONFIG_STD_DEVICES_SETTINGS ""
#define CONFIG_CMDLINE_TAG                      /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 ">"

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE 0x2000

#include <config_cmd_default.h>
#include <configs/chromeos.h>

/* Use the default arch_phys_memset implementation */
#define CONFIG_PHYSMEM

/*
 * Extra bootargs used for direct booting, but not for vboot.
 * - console of the board
 * - debug and earlyprintk: easier to debug; they could be removed later
 */
#define CONFIG_DIRECT_BOOTARGS \
	"console=ttyO0," STRINGIFY(CONFIG_BAUDRATE) \
	" debug earlyprintk cros_debug"

/* Replace default CONFIG_EXTRA_ENV_SETTINGS */
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_CHROMEOS_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0"

/* Replace default CONFIG_BOOTCOMMAND */
#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND CONFIG_NON_VERIFIED_BOOTCOMMAND

/*
 * Undefing CONFIG_CHROMEOS for now since it breaks the
 * build and also because we don't need VBOOT on this platform anyway
 */
#undef CONFIG_SPL
#undef CONFIG_CMD_XIMG
#undef CONFIG_CHROMEOS
#undef CONFIG_OF_LOAD_ENVIRONMENT
#undef CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

#endif /* __configs_chromeos_beaglebone_h__ */
