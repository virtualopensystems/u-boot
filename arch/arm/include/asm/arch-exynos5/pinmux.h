/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __EXYNOS_PINMUX_H
#define __EXYNOS_PINMUX_H

#include <asm/arch/periph.h>

#define PINMUX_FLAG_NONE	0x00000000
#define PINMUX_FLAG_8BIT_MODE	0x00000001	/* SDMMC 8-bit mode */
#define PINMUX_FLAG_SLAVE_MODE	0x00000002	/* SPI slave mode */

/**
 * Configures the pinmux for a particular peripheral.
 *
 * Each gpio can be configured in many different ways (4 bits on exynos)
 * such as "input", "output", "special function", "external interrupt"
 * etc. This function will configure the peripheral pinmux along with
 * pull-up/down and drive strength.
 *
 * @param peripheral	peripheral to be configured
 * @param flags		configure flags
 * @return 0 if ok, -1 on error (e.g. unsupported peripheral)
 */
int exynos_pinmux_config(enum periph_id peripheral, int flags);

#endif
