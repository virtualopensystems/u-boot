/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5250 (EXYNOS5250) board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Enable Serial Flash Support */
#define CONFIG_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH

#include "exynos5-common.h"	/* Common Exynos5 based board configurations */

/* High Level Configuration Options */
#define CONFIG_SMDK5250

/* Console configuration */
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* Enable fdt support for SMDK5250 */
#define CONFIG_DEFAULT_DEVICE_TREE      exynos5250-smdk5250
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE
#define CONFIG_ARCH_DEVICE_TREE		exynos5250

/* Enable booting of fitImage format */
#define CONFIG_FIT
#define CONFIG_FIT_BEST_MATCH

/* input clock of PLL: SMDK5250 has 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ            24000000

#ifndef CONFIG_OF_CONTROL
/* MACH_TYPE_SMDK5250 macro will be removed once added to mach-types */
#define MACH_TYPE_SMDK5250		3774
#define CONFIG_MACH_TYPE		MACH_TYPE_SMDK5250
#endif

/* select serial console configuration */
#define EXYNOS_UART			PERIPH_ID_UART3
#define CONFIG_SERIAL3

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_MSHCI

/* Command definition*/
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_NET
#define CONFIG_CMD_TIME

/* So our flasher can verify that all is well */
#define CONFIG_CRC32_VERIFY

#define CONFIG_CMD_SPI

#define CONFIG_BOOTDELAY		0
#define CONFIG_ZERO_BOOTDELAY_CHECK

/* Generic Device Polling */
#define CONFIG_BOARD_POLL

/* Thermal Management Unit */
#define CONFIG_EXYNOS_TMU
#define CONFIG_CMD_TMU

/* USB */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_EXYNOS
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	3
#define CONFIG_USB_STORAGE

/* I2C */
#define CONFIG_HARD_I2C
#define CONFIG_CMD_I2C
#define CONFIG_I2C_EDID
#define CONFIG_SYS_I2C_SPEED	100000		/* 100 Kbps */
#define CONFIG_DRIVER_S3C24X0_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SLAVE	0x0

/* TPM */
#define CONFIG_CMD_TPM

/* DSIM */
#define DSIM_PHYACCHR_AFC_CTL_VAL	0x3
#define DSIM_PLLCTRL_PMS_VAL		0x3399
#define DSIM_FREQ_BAND			0x8
#define DSIM_PLLTMR_VAL			0x1f4
#define DSIM_ESC_PRESCALAR_VAL		0x90
#define STOP_STATE_CNT_VAL		0xf
#define MAIN_VBP_VAL			0x4
#define STABLE_VFP_VAL			0x4
#define CMD_ALLOW_VAL			0xf
#define MAIN_HBP_VAL			0x4
#define MAIN_HFP_VAL			0x4
#define MAIN_HSA_VAL			0x4
#define MAIN_VSA_VAL			0x4
#define MAIN_VRESOL_VAL			0x320
#define MAIN_HRESOL_VAL			0x500

/* LCD */
#define CONFIG_LCD
#define CONFIG_EXYNOS_LCD
#define LCD_XRES		1280
#define LCD_YRES		800
#define LCD_BPP			LCD_COLOR16

#define CONFIG_MAX77686_POWER

#define CONFIG_TPS65090_POWER
#define CONFIG_TPS65090_I2C_BUS	0x4

#ifndef CONFIG_SPL_BUILD
#define CONFIG_SMARTBAT_POWER
#define CONFIG_BATTERY
#define CONFIG_CMD_BATTERY
#endif

#define CONFIG_VDD_ARM_MV	1300 /* 1.3v */
#define CONFIG_VDD_INT_UV	1012500 /* 1.0125v */
#define CONFIG_VDD_MIF_MV	1000 /* 1.0v */
#define CONFIG_VDD_G3D_MV	1200 /* 1.2v */
#define CONFIG_VDD_LDO2_MV	1500 /* 1.5v */
#define CONFIG_VDD_LDO3_MV	1800 /* 1.8v */
#define CONFIG_VDD_LDO5_MV	1800 /* 1.8v */
#define CONFIG_VDD_LDO10_MV	1800 /* 1.8v */

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"SMDK5250 # "
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC3,115200n8\0"
#define CONFIG_RD_LVL

#define CONFIG_NR_DRAM_BANKS	1

/* Select SPI boot mode support */
#define CONFIG_EXYNOS_SPI_BOOT

#define CONFIG_IDENT_STRING		" for SMDK5250"

/* #define CONFIG_ENV_IS_IN_MMC */
#define CONFIG_SYS_MMC_ENV_DEV		0

/* Ethernet Controllor Driver */
#ifdef CONFIG_CMD_NET
#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#endif /*CONFIG_CMD_NET*/

#ifndef CONFIG_OF_CONTROL
#define CONFIG_SMC911X_BASE		0x5000000
#define CONFIG_ENV_SROM_BANK		1
#endif

/* Enable keyboard */
#define CONFIG_MKBP		/* MKBP protocol */
#define CONFIG_MKBP_KEYB	/* MKBP keyboard input */
#define CONFIG_CMD_MKBP
#define CONFIG_KEYBOARD
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define EXYNOS_DEVICE_SETTINGS	"stdin=serial,mkbp-keyb\0" \
					"stdout=serial,lcd\0" \
					"stderr=serial,lcd\0"

/* Don't load kernel at the very bottom of ram so that it has room when
 * it relocates down. */
#define CONFIG_LOADADDR			0x42000000

/* Keep kernel-passed data below 512MB (i.e. in lowmem with some margin) */
#define CONFIG_SYS_BOOTMAPSZ		(512 << 20)

#define SCRIPT_GENERATE_BOOTARGS "script_generate_bootargs=" \
	"setenv bootargs " \
		"root=/dev/mmcblk${boot_kdevnum}p3 " \
		"rootwait " \
		"ro " \
		"console=ttySAC3,${baudrate} " \
		"cros_legacy " \
		"debug " \
		"earlyprintk " \
		"" \
	"\0"

/* Default boot commands for ChromeOS booting. */
#define CONFIG_BOOTCOMMAND \
	"run script_generate_bootargs; " \
	"mmc rescan ${boot_udevnum}; " \
	"fatload mmc ${boot_udevnum}:c ${loadaddr} ${boot_kernelname}; " \
	"bootm ${loadaddr}; " \
	""

/* Define a basic bootargs for CHROMEOS_BOOTARGS */
#define CONFIG_BOOTARGS "console=ttySAC3," STRINGIFY(CONFIG_BAUDRATE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	SCRIPT_GENERATE_BOOTARGS \
	\
	"boot_udevnum=0\0" \
	"boot_kdevnum=1\0" \
	"boot_kernelname=vmlinuz.uimg.a\0" \
	""

#endif	/* __CONFIG_H */
