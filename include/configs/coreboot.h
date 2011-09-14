/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/ibmpc.h>
/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_COREBOOT
#undef CONFIG_SHOW_BOOT_PROGRESS

/*-----------------------------------------------------------------------
 * Memory layout
 */
#define CHROMEOS_KERNEL_LOADADDR	0x00100000
#define CHROMEOS_KERNEL_BUFSIZE		0x00800000
#define CROSSYSTEM_DATA_ADDRESS \
	(CHROMEOS_KERNEL_LOADADDR + CHROMEOS_KERNEL_BUFSIZE)
#define CROSSYSTEM_DATA_MAXSIZE		0x8000
#define GBB_ADDRESS \
	(CROSSYSTEM_DATA_ADDRESS + CROSSYSTEM_DATA_MAXSIZE)
#define CONFIG_SYS_TEXT_BASE		0x00FC0000

#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_INTEL		1
#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_NM10_AHCI}, \
					{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_MOBILE}


#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#endif

/* Generic TPM interfaced through LPC bus */
#define CONFIG_GENERIC_LPC_TPM
#define CONFIG_TPM_TIS_BASE_ADDRESS        0xfed40000


/*-----------------------------------------------------------------------
 * Real Time Clock Configuration
 */
#define CONFIG_RTC_MC146818
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0
#define CONFIG_SYS_ISA_IO      CONFIG_SYS_ISA_IO_BASE_ADDRESS


/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CONFIG_SERIAL_MULTI
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		1843200
#define CONFIG_SYS_NS16550_RUNTIME_MAPPED
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{300, 600, 1200, 2400, 4800, \
					 9600, 19200, 38400, 115200}
#define CONFIG_SYS_NS16550_COM1	UART0_BASE
#define CONFIG_SYS_NS16550_COM2	UART1_BASE
#define CONFIG_SYS_NS16550_PORT_MAPPED

#define CONFIG_CONSOLE_MUX
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=vga,serial\0" \
                                        "stdout=vga,serial\0" \
                                        "stderr=vga,serial\0"

/* max. 1 IDE bus	*/
#define CONFIG_SYS_IDE_MAXBUS		1
/* max. 1 drive per IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS * 1)

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_ISA_IO_BASE_ADDRESS
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x01f0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x0170
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0x200

#define CONFIG_GENERIC_MMC

#define CONFIG_SUPPORT_VFAT

/************************************************************
 * DISK Partition support
 ************************************************************/
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION


/*-----------------------------------------------------------------------
 * Video Configuration
 */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_COREBOOT
#define CONFIG_VIDEO_SW_CURSOR
#define VIDEO_FB_16BPP_WORD_SWAP
#define CONFIG_I8042_KBD
#define CONFIG_CFB_CONSOLE
#define CONFIG_SYS_CONSOLE_INFO_QUIET

/*-----------------------------------------------------------------------
 * VBoot Configuration.
 */
#define CONFIG_CHROMEOS
#define CHROMEOS_BOOTARGS ""
/* This value is just to get the chromeos library to compile. */
#define CHROMEOS_VBNVCONTEXT_LBA	0

/* Support USB booting */
#define CONFIG_CHROMEOS_USB

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE			115200
#define CONFIG_KGDB_SER_INDEX			2
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT			"boot > "
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + \
						 sizeof(CONFIG_SYS_PROMPT) + \
						 16)
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START		0x00100000
#define CONFIG_SYS_MEMTEST_END			0x01000000
#define CONFIG_SYS_LOAD_ADDR			0x100000
#define CONFIG_SYS_HZ				1000
#define CONFIG_SYS_X86_ISR_TIMER

#define CONFIG_ZBOOT_32

/*-----------------------------------------------------------------------
 * SDRAM Configuration
 */
#define CONFIG_NR_DRAM_BANKS			4

/*-----------------------------------------------------------------------
 * CPU Features
 */

#define CONFIG_SYS_GENERIC_TIMER
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_NUM_IRQS			16

/*-----------------------------------------------------------------------
 * Memory organization:
 * 32kB Stack
 * 16kB Cache-As-RAM @ 0x19200000
 * 256kB Monitor
 * (4MB + Environment Sector Size) malloc pool
 */
#define CONFIG_SYS_STACK_SIZE			(32 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR		(256 * 1024 + 16 * 1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN			(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN			(0x20000 + 4 * 1024 * 1024)

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * FLASH configuration
 */
#define CONFIG_SPI_FLASH
#define CONFIG_NEW_SPI_XFER
#define CONFIG_ICH_SPI
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_NO_FAST_READ
#define CONFIG_SYS_MAX_FLASH_SECT		1
#define CONFIG_SYS_MAX_FLASH_BANKS		1

#define CONFIG_SYS_NO_FLASH	/* means no NOR flash */

/*-----------------------------------------------------------------------
 * Environment configuration
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH

 /* Must match the flash map definition! */
#define CONFIG_ENV_SIZE	  0x4000
#define CONFIG_ENV_OFFSET 0x1c000

/* Might want to keep two copies in the 16K space */
#define CONFIG_ENV_SECT_SIZE 0x1000

/*-----------------------------------------------------------------------
 * PCI configuration
 */
#define CONFIG_PCI

/*-----------------------------------------------------------------------
 * USB configuration
 */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_PCI
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS     12
#define CONFIG_USB_MAX_CONTROLLER_COUNT        2
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX

#define CONFIG_LZMA		1
#define CONFIG_SPLASH_SCREEN	1

	/* FDT stuff */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_CONTROL

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EDITENV
#undef CONFIG_CMD_FPGA
#define CONFIG_CMD_IMI
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ITEST
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_SETGETDCR
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_SPI
#undef CONFIG_CMD_XIMG
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_CBFS
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_USB
#define CONFIG_CMD_TPM

#define CONFIG_BOOTDELAY	-1
#undef  CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND	"run set_bootargs; "\
				"fatload ${devtype} ${devnum}:c 3000000 syslinux/vmlinuz.a; "\
				"zboot 3000000; "

#define CONFIG_EXTRA_ENV_SETTINGS		"devtype=scsi\0"\
						"devnum=0\0"\
						"devname=sda\0"\
						CONFIG_STD_DEVICES_SETTINGS \
						"set_bootargs=setenv bootargs "\
							"console=uart8250,mmio,0xe0401000,115200n8 "\
							"root=/dev/${devname}3 "\
							"init=/sbin/init "\
							"i915.modeset=1 "\
							"rootwait "\
							"ro "\
							"cros_legacy\0"\
						"usb_boot=usb start;"\
							"setenv devtype usb;"\
							"setenv devnum 1;"\
							"setenv devname sdb;"\
							"run bootcmd\0" \
						"mmc_boot=usb start;"\
							"setenv devtype usb;"\
							"setenv devnum 0;"\
							"setenv devname sdb;"\
							"run bootcmd\0" \
						""

#endif	/* __CONFIG_H */
