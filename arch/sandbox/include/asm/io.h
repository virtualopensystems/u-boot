/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 */
#define MAP_NOCACHE	(0)
#define MAP_WRCOMBINE	(0)
#define MAP_WRBACK	(0)
#define MAP_WRTHROUGH	(0)

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags);

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long flags)
{

}

#define __sandbox_write(_a, _v) ({					\
			__typeof__(_v) __v = _v;			\
			__typeof__(_v) *__a =				\
				(__typeof__(_v) *)(uintptr_t)_a;	\
			*__a = __v;					\
		})

#define __sandbox_read(_a, _v) ({					\
			__typeof__(_v) *__a =				\
				(__typeof__(_v) *)(uintptr_t)_a;	\
			_v = *__a;					\
		})

#define writeb(_v, _a)	({ u8 __v = _v; __sandbox_write(_a, __v); })
#define writew(_v, _a)	({ u16 __v = _v; __sandbox_write(_a, __v); })
#define writel(_v, _a)	({ u32 __v = _v; __sandbox_write(_a, __v); })

#define readb(_a)	({ u8 _v; __sandbox_read(_a, _v); })
#define readw(_a)	({ u16 _v; __sandbox_read(_a, _v); })
#define readl(_a)	({ u32 _v; __sandbox_read(_a, _v); })
