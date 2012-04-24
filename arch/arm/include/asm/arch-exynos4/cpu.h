/*
 * (C) Copyright 2010 Samsung Electronics
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

#ifndef _EXYNOS4_CPU_H
#define _EXYNOS4_CPU_H

#include <asm/arch-exynos/cpu.h>

#define EXYNOS4_ADDR_BASE		0x10000000

/* EXYNOS4 */
#define EXYNOS4_GPIO_PART3_BASE		0x03860000
#define EXYNOS4_PRO_ID			0x10000000
#define EXYNOS4_POWER_BASE		0x10020000
#define EXYNOS4_SWRESET			0x10020400
#define EXYNOS4_CLOCK_BASE		0x10030000
#define EXYNOS4_SYSTIMER_BASE		0x10050000
#define EXYNOS4_WATCHDOG_BASE		0x10060000
#define EXYNOS4_MIU_BASE		0x10600000
#define EXYNOS4_DMC0_BASE		0x10400000
#define EXYNOS4_DMC1_BASE		0x10410000
#define EXYNOS4_GPIO_PART2_BASE		0x11000000
#define EXYNOS4_GPIO_PART1_BASE		0x11400000
#define EXYNOS4_FIMD_BASE		0x11C00000
#define EXYNOS4_USBOTG_BASE		0x12480000
#define EXYNOS4_MMC_BASE		0x12510000
#define EXYNOS4_SROMC_BASE		0x12570000
#define EXYNOS4_USBPHY_BASE		0x125B0000
#define EXYNOS4_UART_BASE		0x13800000
#define EXYNOS4_I2C_BASE		0x13860000
#define EXYNOS4_ADC_BASE		0x13910000
#define EXYNOS4_PWMTIMER_BASE		0x139D0000
#define EXYNOS4_MODEM_BASE		0x13A00000
#define EXYNOS4_USBPHY_CONTROL		0x10020704

#define EXYNOS4_GPIO_PART4_BASE		DEVICE_NOT_AVAILABLE

/* Compatibility defines */
#define EXYNOS_POWER_BASE               EXYNOS4_POWER_BASE

#ifndef __ASSEMBLY__

#define SAMSUNG_BASE(device, base)				\
static inline unsigned int samsung_get_base_##device(void)	\
{								\
	return cpu_is_exynos4() ? EXYNOS4_##base : 0;		\
}

SAMSUNG_BASE(adc, ADC_BASE)
SAMSUNG_BASE(clock, CLOCK_BASE)
SAMSUNG_BASE(fimd, FIMD_BASE)
SAMSUNG_BASE(gpio_part1, GPIO_PART1_BASE)
SAMSUNG_BASE(gpio_part2, GPIO_PART2_BASE)
SAMSUNG_BASE(gpio_part3, GPIO_PART3_BASE)
SAMSUNG_BASE(gpio_part4, GPIO_PART4_BASE)
SAMSUNG_BASE(i2c, I2C_BASE)
SAMSUNG_BASE(pro_id, PRO_ID)
SAMSUNG_BASE(mmc, MMC_BASE)
SAMSUNG_BASE(modem, MODEM_BASE)
SAMSUNG_BASE(sromc, SROMC_BASE)
SAMSUNG_BASE(swreset, SWRESET)
SAMSUNG_BASE(timer, PWMTIMER_BASE)
SAMSUNG_BASE(uart, UART_BASE)
SAMSUNG_BASE(usb_phy, USBPHY_BASE)
SAMSUNG_BASE(usb_otg, USBOTG_BASE)
SAMSUNG_BASE(watchdog, WATCHDOG_BASE)
SAMSUNG_BASE(power, POWER_BASE)
#endif

#define EXYNOS_I2C_MAX_CONTROLLERS	1

#endif	/* _EXYNOS4_CPU_H */
