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
#include <asm/arch-coreboot/sysinfo.h>
#include <cros/crossystem_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int
do_netboot_acpi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	chromeos_acpi_t *acpi_table = (chromeos_acpi_t *)lib_sysinfo.vdat_addr;

	acpi_table->vbt7 = 3;

	return 0;
}

U_BOOT_CMD(netboot_acpi, 1, 1, do_netboot_acpi,
		"Fill in Network boot ACPI value", NULL);
