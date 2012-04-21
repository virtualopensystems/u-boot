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
struct exynos4_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank b;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank e2;
	struct s5p_gpio_bank e3;
	struct s5p_gpio_bank e4;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank f2;
	struct s5p_gpio_bank f3;
};

struct exynos4_gpio_part2 {
	struct s5p_gpio_bank j0;
	struct s5p_gpio_bank j1;
	struct s5p_gpio_bank k0;
	struct s5p_gpio_bank k1;
	struct s5p_gpio_bank k2;
	struct s5p_gpio_bank k3;
	struct s5p_gpio_bank l0;
	struct s5p_gpio_bank l1;
	struct s5p_gpio_bank l2;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
	struct s5p_gpio_bank res1[80];

	/* TODO(sjg@chromium.org): Split these into their own part */
	struct s5p_gpio_bank x0;
	struct s5p_gpio_bank x1;
	struct s5p_gpio_bank x2;
	struct s5p_gpio_bank x3;
};

struct exynos4_gpio_part3 {
	struct s5p_gpio_bank z;
};

enum {
	/* GPIO banks are split into this many parts */
	EXYNOS_GPIO_NUM_PARTS	= 3,
	GPIO_MAX_PORT_PART_1	= sizeof(struct exynos4_gpio_part1) /
					sizeof(struct s5p_gpio_bank),
	GPIO_MAX_PORT_PART_2	= GPIO_MAX_PORT_PART_1 +
					sizeof(struct exynos4_gpio_part2) /
					sizeof(struct s5p_gpio_bank),
	GPIO_MAX_PORT_PART_3	= GPIO_MAX_PORT_PART_2 +
					sizeof(struct exynos4_gpio_part3) /
					sizeof(struct s5p_gpio_bank),
	GPIO_MAX_PORT		= GPIO_MAX_PORT_PART_3,
};

#define exynos4_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos4_gpio_part1 *) \
			       EXYNOS4_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS4_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS4_GPIO_PART1_MAX ((sizeof(struct exynos4_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos4_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos4_gpio_part2 *) \
				EXYNOS4_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS4_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS4_GPIO_PART1_MAX)

static inline unsigned int s5p_gpio_base(int nr)
{
	if (!cpu_is_exynos4())
		return 0;
	else if (nr < EXYNOS4_GPIO_PART1_MAX)
		return EXYNOS4_GPIO_PART1_BASE;
	else
		return EXYNOS4_GPIO_PART2_BASE;
}

#endif
#endif
