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

#ifndef _EXYNOS_COMMON_CPU_H
#define _EXYNOS_COMMON_CPU_H

#define DEVICE_NOT_AVAILABLE		0

#define EXYNOS_PRO_ID			0x10000000

/* Address of address of function that copys data from SD or MMC */
#define COPY_BL2_FNPTR_ADDR	0x02020030

#ifndef __ASSEMBLY__
#include <asm/io.h>
/* CPU detection macros */
extern unsigned int s5p_cpu_id;
extern unsigned int s5p_cpu_rev;

static inline int s5p_get_cpu_rev(void)
{
	return s5p_cpu_rev;
}

static inline void s5p_set_cpu_id(void)
{
	s5p_cpu_id = readl(EXYNOS_PRO_ID);
	s5p_cpu_id = (0xC000 | ((s5p_cpu_id & 0x00FFF000) >> 12));

	/*
	 * 0xC200: EXYNOS4210 EVT0
	 * 0xC210: EXYNOS4210 EVT1
	 */
	if (s5p_cpu_id == 0xC200) {
		s5p_cpu_id |= 0x10;
		s5p_cpu_rev = 0;
	} else if (s5p_cpu_id == 0xC210) {
		s5p_cpu_rev = 1;
	}
}

#define IS_SAMSUNG_TYPE(type, id)			\
static inline int cpu_is_##type(void)			\
{							\
	return s5p_cpu_id == id ? 1 : 0;		\
}

IS_SAMSUNG_TYPE(exynos4, 0xc210)
IS_SAMSUNG_TYPE(exynos5, 0xc520)

#endif

#endif	/* _EXYNOS_COMMON_CPU_H */
