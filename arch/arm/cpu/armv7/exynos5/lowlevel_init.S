/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
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
#include <version.h>
#include <asm/arch/cpu.h>

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

	.globl lowlevel_init
lowlevel_init:

	/* use iRAM stack in bl2 */
	ldr	sp, =CONFIG_IRAM_STACK
	stmdb	r13!, {ip,lr}

	/*
	 * It is for EVT0 silicon only and should be removed as soon as
	 * we can deprecate EVT0.
	 */
	ldr    r0, =EXYNOS5_TZPC1_DECPROT1SET
	mov    r1, #0xF        /* [DBGEN, NIDEN, SPIDEN, SPNIDEN] = [1,1,1,1] */
	str    r1, [r0]

	/*
	 * The reason we don't write out the instructions dsb/isb/sev:
	 * While ARM Cortex-A8 supports ARM v7 instruction set (-march=armv7a),
	 * we compile with -march=armv5 to allow more compilers to work.
	 * For U-Boot code this has no performance impact.
	 */
	.word  0xF57FF04F      /* dsb; darn -march=armv5 */
	.word  0xF57FF06F      /* isb; darn -march=armv5 */
	.word  0xE320F004      /* sev; darn -march=armv5 */

	/* check reset status */
	ldr	r0, =(EXYNOS5_POWER_BASE + INFORM1_OFFSET)
	ldr	r1, [r0]

	/* AFTR wakeup reset */
	ldr	r2, =S5P_CHECK_DIDLE
	cmp	r1, r2
	beq	exit_wakeup

	/* LPA wakeup reset */
	ldr	r2, =S5P_CHECK_LPA
	cmp	r1, r2
	beq	exit_wakeup

	/* Sleep wakeup reset */
	ldr	r2, =S5P_CHECK_SLEEP
	cmp	r1, r2
	beq	wakeup_reset

	/* Set the PS-Hold in SPL */
	bl	ps_hold_setup

	/*
	 * If U-boot is already running in RAM, no need to relocate U-Boot.
	 * Memory controller must be configured before relocating U-Boot
	 * in ram.
	 */
	ldr	r0, =0x0ffffff		/* r0 <- Mask Bits*/
	bic	r1, pc, r0		/* pc <- current addr of code */
					/* r1 <- unmasked bits of pc */
	ldr	r2, _TEXT_BASE		/* r2 <- original base addr in ram */
	bic	r2, r2, r0		/* r2 <- unmasked bits of r2*/
	cmp	r1, r2			/* compare r1, r2 */
	beq	1f			/* r0 == r1 then skip sdram init */

	/* init system clock */
	bl	system_clock_init

	/* init power */
	bl	power_init

#if defined(CONFIG_SPL_BUILD)
	/* Memory initialize */
	bl	mem_ctrl_init
#endif

1:
	bl	tzpc_init
	ldmia	r13!, {ip,pc}

wakeup_reset:
	bl	system_clock_init
#if defined(CONFIG_SPL_BUILD)
	bl	mem_ctrl_init
#endif
	bl	tzpc_init

exit_wakeup:
	/* Load return address and jump to kernel */
	ldr	r0, =(EXYNOS5_POWER_BASE + INFORM0_OFFSET)

	/* r1 = physical address of exynos5_cpu_resume function*/
	ldr	r1, [r0]

	/* Jump to kernel */
	mov	pc, r1
	nop
	nop
