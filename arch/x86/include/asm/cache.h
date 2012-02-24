/*
 * Copyright (C) 2004 Eric W. Biederman
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __X86_CACHE_H__
#define __X86_CACHE_H__

/*
 * If CONFIG_SYS_CACHELINE_SIZE is defined use it for DMA alignment.  Otherwise
 * use 64-bytes, a safe default for x86.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	64
#endif

/* The memory clobber prevents the GCC from reordering the read/write order
 * of CR0
 */
static inline unsigned long x86_cache_read_cr0(void)
{
	unsigned long cr0;

	asm volatile ("movl %%cr0, %0" : "=r" (cr0) : : "memory");
	return cr0;
}

static inline void x86_cache_write_cr0(unsigned long cr0)
{
	asm volatile ("movl %0, %%cr0" : : "r" (cr0) : "memory");
}

static inline void wbinvd(void)
{
	asm volatile ("wbinvd" : : : "memory");
}

static inline void invd(void)
{
	asm volatile("invd" : : : "memory");
}

static inline void enable_cache(void)
{
	unsigned long cr0;

	cr0 = x86_cache_read_cr0();
	cr0 &= 0x9fffffff;
	x86_cache_write_cr0(cr0);
}

static inline void disable_cache(void)
{
	/* Disable and write back the cache */
	unsigned long cr0;

	cr0 = x86_cache_read_cr0();
	cr0 |= 0x40000000;
	wbinvd();
	x86_cache_write_cr0(cr0);
	wbinvd();
}

#endif /* __X86_CACHE_H__ */
