/*
 * Copyright (C) 2012 Samsung Electronics
 * Alim Akhtar <alim.akhtar@samsung.com>
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

#include <common.h>
#include <command.h>
#include <asm/arch/exynos-tmu.h>

int do_tmu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int cur_temp;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "curtemp") == 0) {
		if (tmu_monitor(&cur_temp) == -1)
			printf("tmu is in unknow state, temp is invalid\n");
		else
			printf("Current Temp: %u degrees Celsius\n", cur_temp);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	tmu, 2, 1, do_tmu,
	"Thermal Management Unit\n",
	"curtemp - show current cpu temperature in degrees celsius\n"
);
