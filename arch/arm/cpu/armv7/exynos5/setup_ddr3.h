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
