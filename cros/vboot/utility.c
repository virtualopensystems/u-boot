/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/*
 * Implementation of APIs provided by firmware and exported to vboot_reference.
 * They includes debug output, memory allocation, timer and delay, etc.
 */

#include <common.h>
#include <malloc.h>
#include <chromeos/common.h>
#include <chromeos/hda_codec.h>
#include <chromeos/power_management.h>

/* Import the definition of vboot_wrapper interfaces. */
#include <vboot_api.h>

#define UINT32_MAX		(~0UL)
#define TICKS_PER_MSEC		(CONFIG_SYS_HZ / 1000)
#define MAX_MSEC_PER_LOOP	((uint32_t)((UINT32_MAX / TICKS_PER_MSEC) / 2))

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE __BIGGEST_ALIGNMENT__
#endif

static void system_abort(void)
{
	/* Wait for 3 seconds to let users see error messages and reboot. */
	VbExSleepMs(3000);
	cold_reboot();
}

void VbExError(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	system_abort();
}

void VbExDebug(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void *VbExMalloc(size_t size)
{
	void *ptr = memalign(CACHE_LINE_SIZE, size);
	if (!ptr) {
		VbExError("Internal malloc error.");
	}
	return ptr;
}

void VbExFree(void *ptr)
{
	free(ptr);
}

void VbExSleepMs(uint32_t msec)
{
	uint32_t delay, start;

	/*
	 * Can't use entire UINT32_MAX range in the max delay, because it
	 * pushes get_timer() too close to wraparound. So use /2.
	 */
	while(msec > MAX_MSEC_PER_LOOP) {
		VbExSleepMs(MAX_MSEC_PER_LOOP);
		msec -= MAX_MSEC_PER_LOOP;
	}

	delay = msec * TICKS_PER_MSEC;
	start = get_timer(0);

	while (get_timer(start) < delay)
		udelay(100);
}

VbError_t VbExBeep(uint32_t msec, uint32_t frequency)
{
	if (frequency)
		enable_beep();
	else
		disable_beep();

	if (msec > 0) {
		VbExSleepMs(msec);
		disable_beep();
	}

	return VBERROR_SUCCESS;
}

int Memcmp(const void *src1, const void *src2, size_t n)
{
	return memcmp(src1, src2, n);
}

void *Memcpy(void *dest, const void *src, uint64_t n)
{
	return memcpy(dest, src, (size_t) n);
}

void *Memset(void *d, const uint8_t c, uint64_t n)
{
	return memset(d, c, n);
}

