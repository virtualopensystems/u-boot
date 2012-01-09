/*
 * Tegra2 pulse width frequency modulator definitions
 *
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

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pwfm.h>

void pwfm_enable(struct pwfm_ctlr *pwfm, int rate, int pulse_width,
		int freq_divider)
{
	u32 reg;

	/* TODO: Can we use clock_adjust_periph_pll_div() here? */
	clock_start_periph_pll(PERIPH_ID_PWM, CLOCK_ID_SFROM32KHZ, rate);

	reg = PWFM_ENABLE_MASK;
	reg |= pulse_width << PWFM_WIDTH_SHIFT;
	reg |= freq_divider << PWFM_DIVIDER_SHIFT;
	writel(reg, &pwfm->control);
}
