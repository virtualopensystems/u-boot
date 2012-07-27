/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/arch-exynos/spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include <asm/arch/tzpc.h>
#include "setup.h"

static void wakeup_reset(void)
{
	system_clock_init();
#if defined(CONFIG_SPL_BUILD)
	mem_ctrl_init();
#endif
	tzpc_init();
	power_exit_wakeup();
}

void lowlevel_init_c(void)
{
	uint32_t reset_status, pc;

	/*
	 * The reason we don't write out the instructions dsb/isb/sev:
	 * While ARM Cortex-A8 supports ARM v7 instruction set (-march=armv7a),
	 * we compile with -march=armv5 to allow more compilers to work.
	 * For U-Boot code this has no performance impact.
	 */
	__asm__ __volatile__(
#if defined(__thumb__)
	".hword 0xF3BF, 0x8F4F\n"  /* dsb; darn -march=armv5 */
	".hword 0xF3BF, 0x8F6F\n"  /* isb; darn -march=armv5 */
	".hword 0xBF40\n"          /* sev; darn -march=armv5 */
#else
	".word  0xF57FF04F\n"      /* dsb; darn -march=armv5 */
	".word  0xF57FF06F\n"      /* isb; darn -march=armv5 */
	".word  0xE320F004\n"      /* sev; darn -march=armv5 */
#endif
	);

	/* Setup cpu info which is needed to select correct register offsets */
	cpu_info_init();

	reset_status = power_read_reset_status();

	/* AFTR or LPA wakeup reset */
	if (reset_status == S5P_CHECK_DIDLE || reset_status == S5P_CHECK_LPA)
		power_exit_wakeup();
	/* Sleep wakeup reset */
	else if (reset_status == S5P_CHECK_SLEEP)
		wakeup_reset();

	/* Set the PS-Hold in SPL */
	ps_hold_setup();

	/*
	 * If U-boot is already running in RAM, no need to relocate U-Boot.
	 * Memory controller must be configured before relocating U-Boot
	 * in ram.
	 */
	__asm__ __volatile__("mov %[pc], pc\n": [pc]"=r"(pc));
	/* Do sdram init. */
	if ((pc & ~0xffffff) != ((CONFIG_SYS_TEXT_BASE) &  ~0xffffff)) {
		power_init();
		system_clock_init();
#if defined(CONFIG_SPL_BUILD)
		/* Serial initialization only for SPL */
		spl_early_init();
		mem_ctrl_init();
#endif
	}

	tzpc_init();
}
