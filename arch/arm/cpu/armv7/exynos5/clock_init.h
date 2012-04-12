/*
 * Clock initialization routines
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __EXYNOS_CLOCK_INIT_H
#define __EXYNOS_CLOCK_INIT_H

enum {
	MEM_TIMINGS_MSR_COUNT	= 4,
};

/* These are the memory timings for a particular memory type and speed */
struct mem_timings {
	enum ddr_mode mem_type;		/* Memory type */
	unsigned frequency_mhz;		/* Frequency of memory in MHz */

	/* Here follow the timing parameters for the selected memory */
	unsigned bpll_mdiv;
	unsigned bpll_pdiv;
	unsigned bpll_sdiv;
	unsigned pclk_cdrex_ratio;
	unsigned direct_cmd_msr[MEM_TIMINGS_MSR_COUNT];

	unsigned timing_ref;
	unsigned timing_row;
	unsigned timing_data;
	unsigned timing_power;

	/* DQS, DQ, DEBUG offsets */
	unsigned phy0_dqs;
	unsigned phy1_dqs;
	unsigned phy0_dq;
	unsigned phy1_dq;

	unsigned ctrl_force;
	unsigned ctrl_rdlat;
	unsigned ctrl_bstlen;

	unsigned rd_fetch;

	unsigned zq_mode_dds;
	unsigned zq_mode_term;
	unsigned zq_mode_noterm;	/* 1 to allow termination disable */

	unsigned rdlvl_rddata_adj;
	unsigned t_wrrdcmd;
	unsigned memcontrol;
	unsigned memconfig;

	/* Channel and Chip Selection */
	uint8_t dmc_channels;		/* number of memory channels */
	uint8_t chips_per_channel;	/* number of chips per channel */
	uint8_t send_zq_init;		/* 1 to send this command */
};

/**
 * Get the correct memory timings for our selected memory type and speed.
 *
 * This function can be called from SPL or the main U-Boot.
 *
 * @return pointer to the memory timings that we should use
 */
struct mem_timings *clock_get_mem_timings(void);

#endif
