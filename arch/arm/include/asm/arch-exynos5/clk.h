/*
 * (C) Copyright 2012 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#ifndef __ASM_ARM_ARCH_EXYNOS5_CLK_H__
#define __ASM_ARM_ARCH_EXYNOS5_CLK_H__

#include <asm/arch-exynos/clk.h>
#include <asm/arch/pinmux.h>

/*
 * Get the clock divider for mshci controller
 * its a divisor for the Card Interface Unit of the controller
 * and it is used to set the desired bus speed.
 *
 * @param enum periph_id	instance of the mshci controller
 *
 * Return	0 if ok else -1
 */
int get_mshci_clk_div(enum periph_id peripheral);

/*
 * Set mshci controller instances clock drivder
 *
 * @param enum periph_id instance of the mshci controller
 *
 * Return	0 if ok else -1
 */
int clock_set_mshci(enum periph_id peripheral);

#endif  /* __ASM_ARM_ARCH_EXYNOS5_CLK_H__ */
