/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <cros/common.h>
#include <cros/cros_fdtdec.h>
#include <cros/crossystem_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int
do_netboot_acpi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const void *fdt = gd->fdt_blob;
	int node_offset;
	const uint32_t *cell;
	chromeos_acpi_t *acpi_table;

	node_offset = cros_fdtdec_config_node(fdt);
	if (node_offset < 0)
		return 1;

	cell = fdt_getprop(fdt, node_offset, "gnvs-vboot-table", NULL);
	if (!cell) {
		VBDEBUG("Couldn't access gnvs-vboot-table.\n");
		return 1;
	}

	acpi_table = (chromeos_acpi_t *)(uintptr_t)ntohl(*cell);

	acpi_table->vbt7 = 3;

	return 0;
}

U_BOOT_CMD(netboot_acpi, 1, 1, do_netboot_acpi,
		"Fill in Network boot ACPI value", NULL);
