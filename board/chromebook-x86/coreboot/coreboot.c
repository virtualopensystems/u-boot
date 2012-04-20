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
#include <cros/cros_init.h>
#include <cros/crossystem_data.h>
#include <cros/cros_fdtdec.h>
#include <cros/firmware_storage.h>
#include <cros/power_management.h>
#include <spi_flash.h>
#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

unsigned long monitor_flash_len = CONFIG_SYS_MONITOR_LEN;

/*
 * Miscellaneous platform dependent initializations
 */
int cpu_init_f(void)
{
	int ret = get_coreboot_info(&lib_sysinfo);
	if (ret != 0)
		printf("Failed to parse coreboot tables.\n");
	gd->fdt_blob = lib_sysinfo.sys_fdt;
	timestamp_init();
	return ret;
}

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
#if defined CONFIG_CMD_CBFS && defined CONFIG_OF_CONTROL
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
	dtb = malloc(size);
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
cbfs_failed:
#endif /* CONFIG_CMD_CBFS && CONFIG_OF_CONTROL */
	return cros_init();
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

#define MRC_DATA_SIGNATURE (('M' << 0) | ('R' << 8) | ('C' << 16) | ('D' << 24))
#define MRC_DATA_ALIGN     0x1000

struct mrc_data_container {
	u32	mrc_signature;	/* 'MRCD' */
	u32	mrc_data_size;	/* Actual total size of this structure */
	u32	mrc_checksum;	/* IP style checksum */
	u32	reserved;	/* For header alignment */
	u8	mrc_data[0];	/* Variable size, platform/run time dependent */
} __packed;

/**
 * handle_mrc_cache:
 *
 *  - find MRC cache in the SPI flash using the FMAP data
 *  - compare the cache contents passed by coreboot through CBMEM with
 *    the contents saved in the SPI flash
 *  - if the two do not match and the new contents would fit into the
 *    FMAP allocated room - update the contents in the SPI flash.
 */
static void handle_mrc_cache(void)
{
	struct fmap_entry fme;
	struct mrc_data_container *saved_entry;
	struct mrc_data_container *passed_entry;
	struct mrc_data_container *entry, *next_entry;
	u32 passed_size, entry_size, container_size;
	u32 next_offset = 0;

	firmware_storage_t file;

	if (cros_fdtdec_mrc_cache_base(gd->fdt_blob, &fme)) {
		printf("%s: MRC cache not found\n", __func__);
		return;
	}

	passed_entry = (struct mrc_data_container *)lib_sysinfo.mrc_cache;
	passed_size = passed_entry->mrc_data_size;
	entry_size = sizeof(*passed_entry) + passed_size;
	if (passed_size > fme.length) {
		printf("%s: passed entry of %d won't fit into %d\n",
		       __func__, passed_size, fme.length);
		return;
	}

	/* Open firmware storage device. */
	if (firmware_storage_open_spi(&file)) {
		printf("%s: failed to open SPI storage device\n", __func__);
		return;
	}

#ifndef CONFIG_HARDWARE_MAPPED_SPI
	saved_entry = malloc(fme.length);
	if (!saved_entry) {
		printf("%s: failed to allocate %d bytes\n",
		       __func__, fme.length);
		return;
	}
#endif

	if (file.read(&file, fme.offset, fme.length, BT_EXTRA saved_entry)) {
		printf("%s: failed to read %d bytes\n", __func__, fme.length);
		FREE_IF_NEEDED(saved_entry);
		return;
	}

	/* Size is aligned to flash sector size. */
	container_size = passed_entry->mrc_data_size + sizeof(*passed_entry);
	if (container_size & (MRC_DATA_ALIGN - 1UL)) {
		container_size &= ~(MRC_DATA_ALIGN - 1UL);
		container_size += MRC_DATA_ALIGN;
	}

	/* Find the last entry in the region. */
	next_entry = saved_entry;
	do {
		entry = next_entry;
		next_offset += container_size;
		next_entry = (struct mrc_data_container *)
			((u8 *)saved_entry + next_offset);
	} while (next_entry && next_entry->mrc_signature == MRC_DATA_SIGNATURE);

	/* Adjust entry offset back to the entry we want to examine. */
	if (entry->mrc_signature != MRC_DATA_SIGNATURE)
		next_offset = 0;

	if ((entry->mrc_data_size != passed_size) ||
	    memcmp(passed_entry, entry, entry_size)) {
		printf("%s: cached storage mismatch (%d/%d)\n", __func__,
		       entry->mrc_data_size, passed_size);

		/* Erase entire region and start over at entry 0. */
		if ((next_offset + container_size) > fme.length) {
			struct spi_flash *flash = file.context;
			flash->erase(flash, fme.offset, fme.length);
			next_offset = 0;
			printf("%s: region full, erase and start over\n",
			       __func__);
		}

		if ((next_offset + container_size) <= fme.length) {
			if (file.write(&file, fme.offset + next_offset,
				       entry_size, passed_entry))
				printf("%s: write failed!\n", __func__);
		} else {
			printf("%s: passed size too big (%d)\n",
			       __func__, passed_size);
		}
	} else {
		printf("%s: cached storage match\n", __func__);
	}
	FREE_IF_NEEDED(saved_entry);
}

int misc_init_r(void)
{
	handle_mrc_cache();
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
