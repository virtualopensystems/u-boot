/*
 * Copyright (c) 2012 The Chromium OS Authors.
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
#include <fdtdec.h>
#include <asm/arch-coreboot/tables.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <fdt.h>
#include <libfdt.h>
#include <libfdt_env.h>
#include "chromeos.h"

DECLARE_GLOBAL_DATA_PTR;

void chromeos_set_vboot_data_ptr(void)
{
	void *fdt = (void *)gd->fdt_blob;
	int node_offset, addr_cell_len;
	const uint32_t *cell;
	uintptr_t table_addr = (uintptr_t)lib_sysinfo.vdat_addr;
	uint32_t table_addr32;
	uint64_t table_addr64;
	void *table_ptr;

	cell = fdt_getprop(fdt, 0, "#address-cells", NULL);
	if (cell && *cell == 2) {
		addr_cell_len = 8;
		table_addr64 = cpu_to_fdt64(table_addr);
		table_ptr = &table_addr64;
	} else {
		addr_cell_len = 4;
		table_addr32 = cpu_to_fdt32(table_addr);
		table_ptr = &table_addr32;
	}

	node_offset = fdt_path_offset(fdt, "/chromeos-config");
	if (node_offset < 0) {
		printf("Couldn't find /chromeos-config in the FDT.\n");
		return;
	}

	if (fdt_setprop(fdt, node_offset, "gnvs-vboot-table",
			table_ptr, addr_cell_len) < 0) {
		printf("Couldn't set gnvs-vboot-table.\n");
	}
}

