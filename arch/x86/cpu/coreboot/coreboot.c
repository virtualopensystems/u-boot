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

#include <common.h>
#include <config.h>
#include <asm/u-boot-x86.h>
#include <cbfs.h>
#include <flash.h>
#include <fdtdec.h>
#include <malloc.h>
#include <netdev.h>
#include <asm/msr.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch-coreboot/gpio.h>
#include <asm/arch-coreboot/tables.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <coreboot_timestamp.h>
#include <cros/common.h>
#include <cros/crossystem_data.h>
#include <cros/cros_fdtdec.h>
#include <cros/firmware_storage.h>
#include <cros/power_management.h>
#include <spi_flash.h>
#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif
#include "chromeos.h"

DECLARE_GLOBAL_DATA_PTR;

unsigned long monitor_flash_len = CONFIG_SYS_MONITOR_LEN;

/*
 * Miscellaneous platform dependent initializations
 */

#if !defined CONFIG_CMD_CBFS || !defined CONFIG_OF_CONTROL
#error coreboot needs CONFIG_CMD_CBFS and CONFIG_OF_CONTROL enabled.
#endif

static void *find_cbmem_area(void)
{
	int i;
	struct cbmem_entry *cbmem_toc = NULL;

	for (i = 0; i < lib_sysinfo.n_memranges; i++) {
		struct memrange *memrange = &lib_sysinfo.memrange[i];
		/* The CBMEM TOC lives at the last memory area marked
		 * as "tables" in the coreboot memory map. Find it there
		 */
		if (memrange->type == CB_MEM_TABLE)
			cbmem_toc = (struct cbmem_entry *)
				(unsigned long)memrange->base;
	}

	if (!cbmem_toc) {
		printf("Error: Could not find coreboot tables area\n");
		return NULL;
	}

	if (cbmem_toc[0].magic != CBMEM_MAGIC) {
		printf("Error: No CBMEM area found\n");
		return NULL;
	}

	for (i = 0; i < MAX_CBMEM_ENTRIES; i++) {
		if (cbmem_toc[i].magic == CBMEM_MAGIC &&
			cbmem_toc[i].id == CBMEM_ID_RESUME)
			printf("Found usable memory in CBMEM area at 0x%lx\n",
				(unsigned long)cbmem_toc[i].base);
			return (void *)(unsigned long)cbmem_toc[i].base;
	}

	printf("Error: No usable memory found in CBMEM area\n");
	return NULL;
}

static int map_coreboot_serial_to_fdt(void)
{
	void *fdt = (void *)gd->fdt_blob;
	struct cb_serial *serial = lib_sysinfo.serial;
	uint32_t reg[2];
	int serial_offset = fdt_path_offset(fdt, "/serial");
	int ret = 0;
	unsigned long clock_frequency;

	if (serial_offset < 0) {
		/* No aliases node found. Bail out. */
		printf("Couldn't find fdt \"serial\" node.\n");
		return 1;
	}

	/*
	 * If we don't have the a serial console, we don't need to fill
	 * in any properties, so just return.
	 */
	if (!serial)
		return 0;

	/* If there's a serial node and device, populate the node. */
	ret |= fdt_setprop_string(fdt, serial_offset, "compatible", "ns16550");
	reg[0] = cpu_to_fdt32(serial->baseaddr);
	reg[1] = cpu_to_fdt32(0x8);

	ret |= fdt_setprop(fdt, serial_offset, "reg", reg, sizeof(reg));
	ret |= fdt_setprop_cell(fdt, serial_offset, "id", 1);
	ret |= fdt_setprop_cell(fdt, serial_offset, "reg-shift", 1);
	ret |= fdt_setprop_cell(fdt, serial_offset, "baudrate", serial->baud);
	/*
	 * For now, assume an OXPCIE serial adapter
	 * when it's memory mapped, since this is the
	 * only one supported by coreboot.
	 */
	if (serial->type == CB_SERIAL_TYPE_MEMORY_MAPPED)
		clock_frequency = 4000000;
	else
		clock_frequency = 115200;

	ret |= fdt_setprop_cell(fdt, serial_offset,
			"clock-frequency", clock_frequency);

	ret |= fdt_setprop_cell(fdt, serial_offset, "multiplier", 1);

	if (serial->type == CB_SERIAL_TYPE_IO_MAPPED)
		ret |= fdt_setprop_cell(fdt, serial_offset, "io-mapped", 1);

	ret |= fdt_setprop_string(fdt, serial_offset, "status", "ok");

	return !!ret;
}

int cpu_init_f(void)
{
	CbfsFile file;
	int ret = get_coreboot_info(&lib_sysinfo);
	int size;
	void *dtb;

	if (ret != 0)
		printf("Failed to parse coreboot tables.\n");

	file_cbfs_init(0xffffffff);
	if (file_cbfs_result != CBFS_SUCCESS)
		goto cbfs_failed;

	file = file_cbfs_find_uncached(0xffffffff, "u-boot.dtb");
	if (!file)
		goto cbfs_failed;

	size = file->dataLength < 16384 ? file->dataLength : 16384;
	dtb = find_cbmem_area();

	memcpy(dtb, (const void *)(file->data), size);
	gd->fdt_blob = (const void *)dtb;
	if (map_coreboot_serial_to_fdt())
		printf("Couldn't add serial port to FDT.\n");

	timestamp_init();

cbfs_failed:
	return ret;
}

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	CbfsFile file;
	void *dtb;
	u32 size;

	file_cbfs_init(0xffffffff);
	if (file_cbfs_result != CBFS_SUCCESS) {
		printf("%s.\n", file_cbfs_error());
		goto cbfs_failed;
	}
	file = file_cbfs_find("u-boot.dtb");
	if (!file) {
		if (file_cbfs_result != CBFS_FILE_NOT_FOUND)
			printf("%s.\n", file_cbfs_error());
		goto cbfs_failed;
	}
	size = file_cbfs_size(file);
	if (file_cbfs_result != CBFS_SUCCESS) {
		printf("%s.\n", file_cbfs_error());
		goto cbfs_failed;
	}
	dtb = malloc(size + 2048); /* 2K more for console + vdat */
	if (!dtb) {
		printf("Bad allocation!\n");
		goto cbfs_failed;
	}
	if (size != file_cbfs_read(file, dtb, size)) {
		free(dtb);
		printf("%s.\n", file_cbfs_error());
		goto cbfs_failed;
	}
	gd->fdt_blob = dtb;

	chromeos_set_vboot_data_ptr();

	if (map_coreboot_serial_to_fdt())
		printf("Couldn't add serial port to FDT.\n");

cbfs_failed:
	return 0;
}

void show_boot_progress(int val)
{
	outb(val, 0x80);
}


int last_stage_init(void)
{
	return 0;
}

#ifndef CONFIG_SYS_NO_FLASH
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

void setup_pcat_compatibility()
{
}

int misc_init_r(void)
{
	return 0;
}

int board_i8042_skip(void)
{
	cros_gpio_t devsw;

	cros_gpio_fetch(CROS_GPIO_DEVSW, &devsw);
	if (devsw.value)
		return 0;
	return fdtdec_get_config_int(gd->fdt_blob, "skip-i8042", 0);
}

int board_use_usb_keyboard(int boot_mode)
{
	cros_gpio_t devsw;

	/* the keyboard is needed only in developer mode and recovery mode */
	cros_gpio_fetch(CROS_GPIO_DEVSW, &devsw);
	if (!devsw.value && (boot_mode != FIRMWARE_TYPE_RECOVERY))
		return 0;

	/* does this machine have a USB keyboard as primary input ? */
	if (fdtdec_get_config_bool(gd->fdt_blob, "usb-keyboard"))
		return 1;

	return 0;
}

#define MTRR_TYPE_WP          5
#define MTRRcap_MSR           0xfe
#define MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#define MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

int board_final_cleanup(void)
{
	/* Un-cache the ROM so the kernel has one
	 * more MTRR available.
	 *
	 * Coreboot should have assigned this to the
	 * top available variable MTRR.
	 */
	u8 top_mtrr = (rdmsr(MTRRcap_MSR) & 0xff) - 1;
	u8 top_type = rdmsr(MTRRphysBase_MSR(top_mtrr)) & 0xff;

	/* Make sure this MTRR is the correct Write-Protected type */
	if (top_type == MTRR_TYPE_WP) {
		disable_cache();
		wrmsr(MTRRphysBase_MSR(top_mtrr), 0);
		wrmsr(MTRRphysMask_MSR(top_mtrr), 0);
		enable_cache();
	}

	/* Issue SMI to Coreboot to lock down ME and registers */
	printf("Finalizing Coreboot\n");
	outb(0xcb, 0xb2);

	return 0;
}

int do_coldboot(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cold_reboot();
	return (0);
}

U_BOOT_CMD(coldboot, 1, 1, do_coldboot, "Initiate a cold reboot.", "");

int do_poweroff(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	power_off();
	return (0);
}

U_BOOT_CMD(poweroff, 1, 1, do_poweroff, "Switch off power", "");
