/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
/* TODO(thutt): Replace '__u32' with 'uint32_t' */

#ifndef __SANDBOX_API_H
#define __SANDBOX_API_H

#include <asm/types.h>

#include "asm/sandbox-spi.h"
#include "asm/sandbox-mmc.h"

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(_a) (sizeof(_a) / sizeof(_a[0]))
#endif

#define SANDBOX_SHM_START     0x10000000
#define SANDBOX_SHM_ADDRESS   ((void *)(SANDBOX_SHM_START))
#define SANDBOX_SHM_KEY       ((key_t)0xbeefcafe)
#define SANDBOX_SHM_SIZE      (24U * 1024 * 1024)
#define SANDBOX_SHM_FLAGS     (IPC_CREAT | IPC_EXCL | S_IRWXU)
#define SANDBOX_PAGE_SIZE     4096
#define SANDBOX_RESERVE_PAGES 16

#define ASSERT_ON_COMPILE(e)			\
	do {					\
		enum { m = ((e) ? 1 : -1) };	\
		typedef char f[m];		\
	} while (0)

/*
 * This address layout must correspond to the one in README.text, and
 * in sandbox.dts.
 */
#define SANDBOX_MEMORY_LAYOUT			\
	SML(FREE, 0x16df)			\
	SML(RW_A, 0x1)				\
	SML(RW_B, 0x1)				\
	SML(KERNEL, 0x100)			\
	SML(CROS_SYSTEM_DATA, 0x8)		\
	SML(GOOGLE_BINARY_BLOCK, 0x8)		\
	SML(GPIO, 0x1)

enum sandbox_memory_region {
#define SML(_id, _pages) SML_##_id,
	SANDBOX_MEMORY_LAYOUT
#undef SML
	N_SANDBOX_MEMORY_REGIONS
};

#define DEVICES					\
	D(SPI,	"SPI FLASH ROM")		\
	D(KEYBOARD, "Keyboard")			\
	D(MMC0, "MMC0")				\
	D(MMC1, "MMC1")

enum device_t {
#define D(n, s) SB_##n,
	DEVICES
#undef D
	SB_N_DEVICES
};

struct doorbell_command_t {
	volatile __u32 doorbell; /* Set to non-zero to start command.
				  * Reset to zero when command
				  * completed.
				  */
	__u32 result;		/*  0 -> success
				 * !0 -> failure
				*/
	__u32 device_id;	/* device_t value */
	__u32 command_data[32]; /* device-specific values */
	unsigned char dbc_buf[16384]; /* data i/o area */
};

/*
 * Addresses reported in this structure are based at the shared memory
 * address (0x10000000), not at zero.
 */
struct doorbell_t {
	__u32 exit;
	struct spi_t spi;
	struct mmc_t mmc[2];
	struct doorbell_command_t cmd;
};

static inline struct doorbell_t *sandbox_get_doorbell(void)
{
	void *addr = (((unsigned char *)SANDBOX_SHM_ADDRESS +
		       SANDBOX_SHM_SIZE) -
		      (SANDBOX_RESERVE_PAGES * SANDBOX_PAGE_SIZE));

	ASSERT_ON_COMPILE((SANDBOX_PAGE_SIZE & ~SANDBOX_PAGE_SIZE) == 0);
	ASSERT_ON_COMPILE(sizeof(struct doorbell_t) <=
			  SANDBOX_RESERVE_PAGES * SANDBOX_PAGE_SIZE);
	return (struct doorbell_t *)addr;
}

static inline struct doorbell_command_t *sandbox_get_doorbell_command(void)
{
	return &sandbox_get_doorbell()->cmd;
}

static inline void sandbox_ring_doorbell(void)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->doorbell = 1;
	while (dbc->doorbell)
		;
}

static inline unsigned sandbox_in_shared_memory(const void *p)
{
	return (SANDBOX_SHM_ADDRESS <= p &&
		p < SANDBOX_SHM_ADDRESS + SANDBOX_SHM_SIZE);
}

static inline void *sandbox_region_address(enum sandbox_memory_region region)
{
	/*
	 * TODO(thutt@chromium.org): This should eventually not be an
	 * inline function so that code size can be reduced.
	 */
	unsigned n_bytes[] = {
#define SML(_id, _pages) _pages,
		SANDBOX_MEMORY_LAYOUT
#undef SML
	};
	unsigned char *start = SANDBOX_SHM_ADDRESS;
	unsigned i = 0;
	while (i < region) {
		start += n_bytes[i] * SANDBOX_PAGE_SIZE;
		++i;
	}
	return start;
}
#endif
