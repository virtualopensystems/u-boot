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

#ifndef __CONFIG_COREBOOT_H
#define __CONFIG_COREBOOT_H

#include <asm/ibmpc.h>

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_COREBOOT
#define CONFIG_SHOW_BOOT_PROGRESS
#define BUILD_IDE_STUFF 0

#ifdef CONFIG_FACTORY_IMAGE
#define BUILD_CMD_LINE_STUFF 1
#define BUILD_NETWORK_STUFF  1
#define BUILD_PART_FS_STUFF  1
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial,cbmem\0" \
					"stderr=vga,serial,cbmem\0"
#else
#define BUILD_CMD_LINE_STUFF 0
#define BUILD_NETWORK_STUFF  0
#define BUILD_PART_FS_STUFF  0
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=serial,cbmem\0" \
					"stderr=vga,serial,cbmem\0"
#endif

/* FDT support */
#define CONFIG_OF_LIBFDT	/* Device tree support */
#define CONFIG_OF_CONTROL	/* Use the device tree to set up U-Boot */
#define CONFIG_DEFAULT_DEVICE_TREE      link

/*-----------------------------------------------------------------------
 * Memory layout
 */
/* TODO(sjg): Move these two to the fdt */
#define CONFIG_VBGLOBAL_BASE		0x01100000
#define CONFIG_SYS_TEXT_BASE		0x01110000

/* SATA AHCI storage */

#define CONFIG_SCSI_AHCI

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_INTEL		1
#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_NM10_AHCI},	      \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_MOBILE}, \
	{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_SERIES6}, \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_PANTHERPOINT_AHCI_MOBILE}


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
#define CONFIG_OF_SERIAL
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
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
#define CONFIG_CBMEM_CONSOLE

/* turn on command-line edit/hist/auto */
#if BUILD_CMD_LINE_STUFF
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTOCOMPLETE
#endif

/* Enable keyboard */
#define CONFIG_MKBP		/* MKBP protocol */
#define CONFIG_MKBP_LPC		/* MKBP over LPC */
#define CONFIG_CMD_MKBP

#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_STDIO_DEREGISTER

#if BUILD_IDE_STUFF
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
#endif

#if BUILD_PART_FS_STUFF
#define CONFIG_SUPPORT_VFAT

/************************************************************
 * DISK Partition support
 ************************************************************/
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION
#endif


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
#define V_PROMPT			"boot > "
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		V_PROMPT
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SYS_64BIT_LBA

#if BUILD_CMD_LINE_STUFF
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#endif
/*
 * Increasing the size of the IO buffer as default nfsargs size is more
 *  than 256 and so it is not possible to edit it
 */
#define CONFIG_SYS_CBSIZE		( 256 * 2 ) /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x00100000
#define CONFIG_SYS_MEMTEST_END		0x01000000

#define CONFIG_SYS_LOAD_ADDR		0x100000
#define CONFIG_SYS_HZ			1000
#define CONFIG_NO_RESET_CODE

/* coreboot tweaking */
#define CONFIG_ZBOOT_32

/*-----------------------------------------------------------------------
 * SDRAM Configuration
 */
#define CONFIG_NR_DRAM_BANKS		4

/*-----------------------------------------------------------------------
 * CPU Features
 */

#define CONFIG_SYS_GENERIC_TIMER
#define CONFIG_SYS_X86_ISR_TIMER
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_NUM_IRQS		16

/*-----------------------------------------------------------------------
 * Memory organization:
 * 32kB Stack
 * 16kB Cache-As-RAM @ 0x19200000
 * 256kB Monitor
 * (4MB + Environment Sector Size) malloc pool
 */
#define CONFIG_SYS_STACK_SIZE		(32 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR		(256 * 1024 + 16 * 1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(0x20000 + 4 * 1024 * 1024)

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * FLASH configuration
 */
#define CONFIG_ICH_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_NEW_SPI_XFER
#define CONFIG_SPI_FLASH_NO_FAST_READ
#define CONFIG_SYS_MAX_FLASH_SECT	1
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_NO_FLASH

/*-----------------------------------------------------------------------
 * Environment configuration
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH

 /* Must match the flash map definition! */
#define CONFIG_ENV_SIZE	  0x4000
#define CONFIG_ENV_OFFSET 0x39c000

/* Might want to keep two copies in the 16K space */
#define CONFIG_ENV_SECT_SIZE 0x1000

/*-----------------------------------------------------------------------
 * PCI configuration
 */
#define CONFIG_PCI
/* x86 GPIOs are accessed through a PCI device */
#define CONFIG_INTEL_ICH6_GPIO

/*-----------------------------------------------------------------------
 * USB configuration
 */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_PCI
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS     12
#define CONFIG_USB_MAX_CONTROLLER_COUNT        2
#define CONFIG_USB_STORAGE
#define CONFIG_USB_KEYBOARD

#if BUILD_NETWORK_STUFF
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#endif

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#if BUILD_CMD_LINE_STUFF
#include <config_cmd_default.h>

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_SF
#define CONFIG_CMD_TIME
#define CONFIG_CMD_TPM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#endif

/* These also control whether some generic support is built. */
#define CONFIG_CMD_SPI
#define CONFIG_CMD_USB
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_ZBOOT
#define CONFIG_CMD_CBFS

#if BUILD_NETWORK_STUFF
#define CONFIG_CMD_NET
#else
#undef CONFIG_CMD_NET
#endif

#define CONFIG_HARDWARE_MAPPED_SPI
#define CONFIG_INTEL_CORE_ARCH	/* Sandy bridge and ivy bridge chipsets. */

/* Board specific late time init */
#define CONFIG_MISC_INIT_R

/* Boot options */

#define CONFIG_BOOTDELAY     0
#define CONFIG_BOOTARGS		""

#ifdef CONFIG_FACTORY_IMAGE
#undef CONFIG_ZERO_BOOTDELAY_CHECK

/* TODO: Remove the second 'net_boot'. See crosbug/p/11152 */
#define CONFIG_BOOTCOMMAND \
	"netboot_acpi; "\
	"run set_netbootargs; "\
	"usb start; "\
	"setenv tftpblocksize 1408; "\
	"run net_boot; "\
	"setenv tftpblocksize 512; "\
	"run net_boot; "\
	"setenv devnum 0; "\
	"run fallback_usb; "\
	"setenv devnum 1; "\
	"run fallback_usb; "

#define CONFIG_NETBOOT_EXTRA_ENV \
	"set_netbootargs=setenv bootargs "\
		"root=/dev/ram0 "\
		"rw "\
		"init=/sbin/init "\
		"i915.modeset=1 "\
		"cros_legacy "\
		"ramdisk_size=409600\0"\
	"net_boot=setenv loadaddr 0x100000; "\
		"setenv bootfile uImage; "\
		"dhcp; "\
		"setenv loadaddr 0x12008000; "\
		"setenv bootfile rootImg; "\
		"dhcp; "\
		"bootm 0x100000 0x12008000\0"\
	"fallback_usb=setenv devtype usb; "\
		"setenv devname sdb; "\
		"run set_bootargs; "\
		"setenv bootargs ${bootargs} cros_factory_install; "\
		"if fatload ${devtype} ${devnum}:c 3000000 syslinux/vmlinuz.A; then "\
			"zboot 3000000; "\
			"fi\0"
#else
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTCOMMAND \
	"run set_bootargs; "\
	"setenv bootargs ${bootargs} console=uart8250,mmio,0xe0401000,115200n8; "\
	"fatload ${devtype} ${devnum}:c 3000000 syslinux/vmlinuz.a; "\
	"zboot 3000000; "
#define CONFIG_NETBOOT_EXTRA_ENV ""
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"devtype=scsi\0"\
	"devnum=0\0"\
	"devname=sda\0"\
	CONFIG_STD_DEVICES_SETTINGS \
	"set_bootargs=setenv bootargs "\
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
	CONFIG_NETBOOT_EXTRA_ENV

#endif	/* __CONFIG_H */
