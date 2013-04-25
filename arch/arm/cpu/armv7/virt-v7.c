/*
 * (C) Copyright 2013
 * Andre Przywara, Linaro
 *
 * Routines to transition ARMv7 processors from secure into non-secure state
 * needed to enable ARMv7 virtualization for current hypervisors
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
#include <asm/armv7.h>
#include <asm/gic.h>
#include <asm/io.h>

static unsigned int read_id_pfr1(void)
{
	unsigned int reg;

	asm("mrc p15, 0, %0, c0, c1, 1\n" : "=r"(reg));
	return reg;
}

static int get_gicd_base_address(unsigned int *gicdaddr)
{
#ifdef CONFIG_ARM_GIC_BASE_ADDRESS
	*gicdaddr = CONFIG_ARM_GIC_BASE_ADDRESS + GIC_DIST_OFFSET;
	return 0;
#else
	unsigned midr;
	unsigned periphbase;

	/* check whether we are an Cortex-A15 or A7.
	 * The actual HYP switch should work with all CPUs supporting
	 * the virtualization extension, but we need the GIC address,
	 * which we know only for sure for those two CPUs.
	 */
	asm("mrc p15, 0, %0, c0, c0, 0\n" : "=r"(midr));
	switch (midr & MIDR_PRIMARY_PART_MASK) {
	case MIDR_CORTEX_A9_R0P1:
	case MIDR_CORTEX_A15_R0P0:
	case MIDR_CORTEX_A7_R0P0:
		break;
	default:
		return NONSEC_ERR_NO_GIC_ADDRESS;
	}

	/* get the GIC base address from the CBAR register */
	asm("mrc p15, 4, %0, c15, c0, 0\n" : "=r" (periphbase));

	/* the PERIPHBASE can be mapped above 4 GB (lower 8 bits used to
	 * encode this). Bail out here since we cannot access this without
	 * enabling paging.
	 */
	if ((periphbase & 0xff) != 0)
		return NONSEC_ERR_GIC_ADDRESS_ABOVE_4GB;

	*gicdaddr = periphbase + GIC_DIST_OFFSET;

	return 0;
#endif
}

enum nonsec_virt_errors armv7_switch_nonsec(void)
{
	unsigned int reg, ret;
	unsigned int gicdaddr = 0;
	unsigned itlinesnr, i;

	/* check whether the CPU supports the security extensions */
	reg = read_id_pfr1();
	if ((reg & 0xF0) == 0)
		return NONSEC_ERR_NO_SEC_EXT;

	/* the SCR register will be set directly in the monitor mode handler,
	 * according to the spec one should not tinker with it in secure state
	 * in SVC mode. Do not try to read it once in non-secure state,
	 * any access to it will trap.
	 */

	ret = get_gicd_base_address(&gicdaddr);
	if (ret != 0)
		return ret;

	/* enable the GIC distributor */
	writel(readl(gicdaddr + GICD_CTLR) | 0x03, gicdaddr + GICD_CTLR);

	/* TYPER[4:0] contains an encoded number of available interrupts */
	itlinesnr = readl(gicdaddr + GICD_TYPER) & 0x1f;

	/* set all bits in the GIC group registers to one to allow access
	 * from non-secure state
	 */
	for (i = 0; i <= itlinesnr; i++)
		writel((unsigned)-1, gicdaddr + GICD_IGROUPRn + 4 * i);

	/* call the non-sec switching code on this CPU */
	_nonsec_init();

	return NONSEC_VIRT_SUCCESS;
}
