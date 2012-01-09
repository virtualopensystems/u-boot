/*
 * Tegra pulse width frequency modulator definitions
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

#ifndef __ASM_ARCH_TEGRA_PWFM_H
#define __ASM_ARCH_TEGRA_PWFM_H

/* This is a single PWFM channel */
struct pwfm_ctlr {
	uint control;		/* Control register */
};

/* PWM_CONTROLLER_PWM_CSR_0/1/2/3_0 */
#define PWFM_ENABLE_SHIFT	31
#define PWFM_ENABLE_MASK	(0x1 << PWFM_ENABLE_SHIFT)

#define PWFM_WIDTH_SHIFT	16
#define PWFM_WIDTH_MASK		(0x7FFF << PWFM_WIDTH_SHIFT)

#define PWFM_DIVIDER_SHIFT	0
#define PWFM_DIVIDER_MASK	(0x1FFF << PWFM_DIVIDER_SHIFT)

/**
 * Program the PWFM with the given parameters.
 *
 * @param pwfm		Pointer to PWFM register
 * @param rate		Clock rate to use for PWFM
 * @param pulse_width	high pulse width: 0=always low, 1=1/256 pulse high,
 *			n = n/256 pulse high
 * @param freq_divider	frequency divider value (1 to use rate as is)
 */
void pwfm_enable(struct pwfm_ctlr *pwfm, int rate, int pulse_width,
		int freq_divider);

#endif	/* __ASM_ARCH_TEGRA_PWFM_H */
