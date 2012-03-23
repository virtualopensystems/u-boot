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
 */

#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#include <asm/arch-exynos/gpio.h>

#ifndef __ASSEMBLY__
struct exynos5_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank a2;
	struct s5p_gpio_bank b0;
	struct s5p_gpio_bank b1;
	struct s5p_gpio_bank b2;
	struct s5p_gpio_bank b3;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank c2;
	struct s5p_gpio_bank c3;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
	char res1[0x980];
	struct s5p_gpio_bank x0;
	struct s5p_gpio_bank x1;
	struct s5p_gpio_bank x2;
	struct s5p_gpio_bank x3;
};

struct exynos5_gpio_part2 {
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank g0;
	struct s5p_gpio_bank g1;
	struct s5p_gpio_bank g2;
	struct s5p_gpio_bank h0;
	struct s5p_gpio_bank h1;
};

struct exynos5_gpio_part3 {
	struct s5p_gpio_bank v0;
	struct s5p_gpio_bank v1;
	struct s5p_gpio_bank v2;
	struct s5p_gpio_bank v3;
	struct s5p_gpio_bank res1[0x20];
	struct s5p_gpio_bank v4;
};

struct exynos5_gpio_part4 {
	struct s5p_gpio_bank z;
};

#define exynos5_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos5_gpio_part1 *) \
			       EXYNOS5_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS5_GPIO_PART1_MAX ((sizeof(struct exynos5_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5_gpio_part2 *) \
				EXYNOS5_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART1_MAX)

#define EXYNOS5_GPIO_PART2_MAX ((sizeof(struct exynos5_gpio_part2) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part3_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5_gpio_part3 *) \
				EXYNOS5_GPIO_PART3_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART3_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART2_MAX)

static inline unsigned int s5p_gpio_base(int nr)
{
	if (!cpu_is_exynos5())
		return 0;
	else if (nr < EXYNOS5_GPIO_PART1_MAX)
			return EXYNOS5_GPIO_PART1_BASE;
	else if (nr < EXYNOS5_GPIO_PART2_MAX)
		return EXYNOS5_GPIO_PART2_BASE;
	else
		return EXYNOS5_GPIO_PART3_BASE;
}

#endif
#endif
