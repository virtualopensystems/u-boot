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
#include <ns16550.h>
#include <asm/msr.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch-coreboot/tables.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <asm/arch-coreboot/timestamp.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */

#if !defined CONFIG_CMD_CBFS || !defined CONFIG_OF_CONTROL
#error coreboot needs CONFIG_CMD_CBFS and CONFIG_OF_CONTROL enabled.
#endif

/*
 * Set this to a nonzero value to make sure there is at least so many
 * thousands of tsc clocks between port 80 accesses. Set it to 4000 for the
 * Google EC port 80 implementation.
 */
#define MIN_PORT80_KCLOCKS_DELAY	0

#ifdef CONFIG_OF_CBFS
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
#endif

static int map_coreboot_serial_to_fdt(void)
{
	void *fdt = (void *)gd->fdt_blob;
	struct cb_serial *serial = lib_sysinfo.serial;
	uint32_t reg[2];
	int serial_offset;
	int ret = 0;
	unsigned long clock_frequency;

	serial_offset = fdt_path_offset(fdt, "/serial");
	if (serial_offset < 0) {
		/* No aliases node found. Bail out. */
		printf("Couldn't find fdt \"serial\" node.\n");
		return 1;
	}

	/* If coreboot has serial data, use it, otherwise we are done */
	if (!serial)
		return 0;

	/* If there's a serial node and device, populate the node. */
	reg[0] = cpu_to_fdt32(serial->baseaddr);
	reg[1] = cpu_to_fdt32(0x8);

	ret |= fdt_setprop(fdt, serial_offset, "reg", reg, sizeof(reg));
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

	ret |= fdt_setprop_cell(fdt, serial_offset, "io-mapped",
			serial->type == CB_SERIAL_TYPE_IO_MAPPED);

	ret |= fdt_setprop_string(fdt, serial_offset, "status", "okay");

	return !!ret;
}

#ifdef CONFIG_OF_CBFS
/**
 * Load the fdt from CBFS
 *
 * @return 0 if ok, else error
 */
static int load_fdt_from_cbfs(void)
{
	CbfsFile file;
	int size;
	void *dtb;

	file_cbfs_init(0xffffffff);
	if (file_cbfs_result != CBFS_SUCCESS)
		return -1;

	file = file_cbfs_find_uncached(0xffffffff, "u-boot.dtb");
	if (!file)
		return -1;

	size = file->dataLength < 16384 ? file->dataLength : 16384;
	dtb = find_cbmem_area();

	memcpy(dtb, (const void *)(file->data), size);
	gd->fdt_blob = (const void *)dtb;

	return 0;
}
#endif

int cpu_init_f(void)
{
	int ret;

	ret = get_coreboot_info(&lib_sysinfo);
	if (ret != 0)
		printf("Failed to parse coreboot tables.\n");

#ifdef CONFIG_OF_CBFS
	/*
	 * For cold boot, get the fdt from CBFS. For warm boot it will come
	 * from memory using CONFIG_OF_SEPARATE.
	 */
	if (gd->flags & GD_FLG_COLD_BOOT) {
		ret = load_fdt_from_cbfs();
		if (ret)
			printf("Failed to find fdt in CBFS.\n");
	}
#endif

	timestamp_init();

	return ret;
}

int board_early_init_f(void)
{
	if (map_coreboot_serial_to_fdt())
		printf("Couldn't add serial port to FDT.\n");

	return 0;
}

int board_early_init_r(void)
{
	if (map_coreboot_serial_to_fdt())
		printf("Couldn't add serial port to FDT.\n");
	return 0;
}

void show_boot_progress(int val)
{
#if MIN_PORT80_KCLOCKS_DELAY
	static uint32_t prev_stamp __attribute__((section(".data")));
	static uint32_t base __attribute__((section(".data")));

	/*
	 * Scale the time counter reading to avoid using 64 bit arithmetics.
	 * Can't use get_timer() here becuase it could be not yet
	 * initialized or even implemented.
	 */
	if (!prev_stamp) {
		base = rdtsc() / 1000;
		prev_stamp = 0;
	} else {
		uint32_t now;

		do {
			now = rdtsc() / 1000 - base;
		} while (now < (prev_stamp + MIN_PORT80_KCLOCKS_DELAY));
		prev_stamp = now;
	}
#endif
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

int misc_init_r(void)
{
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

void panic_puts(const char *str)
{
	NS16550_t port = (NS16550_t)0x3f8;

	NS16550_is_io_mapped(1);
	NS16550_init(port, 1);
	while (*str)
		NS16550_putc(port, *str++);
}
