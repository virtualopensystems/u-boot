/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of per-board codec beeping */

#include <common.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/i8254.h>
#include <pci.h>
#include <cros/hda_codec.h>
#include <cros/pcbeep.h>

#define PPC_PORTB       0x61
#define PORTB_BEEP_ENABLE 0x3
#define PIT_HZ 1193180L

/* Timer 2 legacy PC beep functions */
static void enable_beep_timer2(uint32_t frequency)
{
	uint32_t countdown;

	if (!frequency)
		return;

	countdown = PIT_HZ / frequency;

	outb(PIT_CMD_CTR2 | PIT_CMD_BOTH | PIT_CMD_MODE3,
	     PIT_BASE + PIT_COMMAND);
	outb(countdown & 0xff, PIT_BASE + PIT_T2);
	outb((countdown >> 8) & 0xff , PIT_BASE + PIT_T2);
	outb(inb(PPC_PORTB) | PORTB_BEEP_ENABLE, PPC_PORTB);
}

static void disable_beep_timer2(void)
{
	outb(inb(PPC_PORTB) & !PORTB_BEEP_ENABLE, PPC_PORTB);
}

void enable_beep(uint32_t frequency)
{
	fdtdec_get_config_int(gd->fdt_blob, "hda_codec_beep", 0) ? \
	   enable_beep_hda(frequency) : enable_beep_timer2(frequency);
}

void disable_beep(void)
{
	fdtdec_get_config_int(gd->fdt_blob, "hda_codec_beep", 0) ? \
	   disable_beep_hda() : disable_beep_timer2();
}
