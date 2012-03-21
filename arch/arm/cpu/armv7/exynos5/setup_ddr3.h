/*
 * Machine Specific Values for SMDK5250 board based on Exynos5
 *
 * Copyright (C) 2012 Samsung Electronics
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

#ifndef _SMDK5250_SETUP_DDR3_H
#define _SMDK5250_SETUP_DDR3_H

#ifdef CDREX_800
#define DDR3_BPLL_MDIV			0x64
#define DDR3_BPLL_PDIV			0x3
#define DDR3_BPLL_SDIV			0x0

#define DDR3_PCLK_CDREX_RATIO		0x5
#define DDR3_CTRL_FORCE			0x2a

#define DDR3_DIRECT_CMD_MRS1		0x00020018
#define DDR3_DIRECT_CMD_MRS2		0x00030000
#define DDR3_DIRECT_CMD_MRS3		0x00010042
#define DDR3_DIRECT_CMD_MRS4		0x00000D70

#define DDR3_DMC_TIMINGROW_VAL		0x8D46650F
#define DDR3_DMC_TIMINGDATA_VAL		0x4740180B
#define DDR3_DMC_TIMINGPOWER_VAL	0x41000A44

#define DDR3_CTRL_RDLAT			0x0b

/* DQS, DQ, DEBUG offsets */
#define DDR3_PHY0_DQS			0x0f0f0f0f
#define DDR3_PHY1_DQS			0x1d1f1f19
#define DDR3_PHY1_DQ			0x88888888

#define DDR3_RD_FETCH			0x3
#else
#define DDR3_BPLL_MDIV			0x185
#define DDR3_BPLL_PDIV			0x7
#define DDR3_BPLL_SDIV			0x1

#define DDR3_PCLK_CDREX_RATIO		0x4
#define DDR3_CTRL_FORCE			0x33

#define DDR3_DIRECT_CMD_MRS1		0x00020010
#define DDR3_DIRECT_CMD_MRS2		0x00030000
#define DDR3_DIRECT_CMD_MRS3		0x00010042
#define DDR3_DIRECT_CMD_MRS4		0x00000B50

#define DDR3_DMC_TIMINGROW_VAL		0x7645644d
#define DDR3_DMC_TIMINGDATA_VAL		0x45414709
#define DDR3_DMC_TIMINGPOWER_VAL	0x3a000a3c

#define DDR3_CTRL_RDLAT			0x09

/* DQS, DQ, DEBUG offsets */
#define DDR3_PHY0_DQS			0x08080808
#define DDR3_PHY1_DQS			0x08080808
#define DDR3_PHY1_DQ			0x00080808
#define DDR3_RD_FETCH			0x2
#endif

#define DDR3_PHY0_DQ			0x08080808

#define DDR3_DMC_TIMINGREF_VAL		0x000000BB

#define DDR3_ZQ_MODE_DDS_VAL		(0x7 << 24)
#define DDR3_ZQ_MODE_TERM_VAL		(0x2 << 21)

#define DDR3_RDLVL_RDDATA_ADJ		0xFF00
#define DDR3_T_RDDATA_MARGIN		0x2

#define DDR3_DMC_MEMCONTROL_VAL ( \
	DMC_MEMCONTROL_CLK_STOP_DISABLE | \
	DMC_MEMCONTROL_DPWRDN_DISABLE | \
	DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE | \
	DMC_MEMCONTROL_TP_DISABLE | \
	DMC_MEMCONTROL_DSREF_DISABLE | \
	DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) | \
	DMC_MEMCONTROL_MEM_TYPE_DDR3 | \
	DMC_MEMCONTROL_MEM_WIDTH_32BIT | \
	DMC_MEMCONTROL_NUM_CHIP_1 | \
	DMC_MEMCONTROL_BL_8 | \
	DMC_MEMCONTROL_PZQ_DISABLE | \
	DMC_MEMCONTROL_MRR_BYTE_7_0 \
)
#define DDR3_DMC_MEMCONFIG_VAL	( \
	DMC_MEMCONFIGx_CHIP_MAP_INTERLEAVED | DMC_MEMCONFIGx_CHIP_COL_10 | \
	DMC_MEMCONFIGx_CHIP_ROW_15 | DMC_MEMCONFIGx_CHIP_BANK_8)

#define DDR3_CTRL_BSTLEN		0x08

#define DDR3_ZQ_MODE_NOTERM		(0 << 19)

#endif
