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

/*
 * Peripherals requiring pinmux configuration. List will
 * grow with support for more devices getting added.
 */
enum {
	EXYNOS_UART0,
	EXYNOS_UART1,
	EXYNOS_UART2,
	EXYNOS_UART3,
	EXYNOS_SDMMC0_8BIT,
	EXYNOS_SDMMC0,
	EXYNOS_SDMMC1,
	EXYNOS_SDMMC2_8BIT,
	EXYNOS_SDMMC2,
	EXYNOS_SDMMC3,
	EXYNOS_SMC911X,
};

/**
 * Configures the pinmux for a particular peripheral.
 *
 * Each gpio can be configured in many different ways (4 bits on exynos)
 * such as "input", "output", "special function", "external interrupt"
 * etc. This function will configure the peripheral pinmux along with
 * pull-up/down and drive strength.
 *
 * @param config	peripheral to be configured
 * @return 0 if ok, -1 on error (e.g. unsupported peripheral)
 */
int exynos_pinmux_config(int peripheral);

#endif
