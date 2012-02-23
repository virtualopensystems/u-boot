/*
 * DDR3 mem setup file for SMDK5250 board based on EXYNOS5
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

#include <config.h>
#include <asm/io.h>
#include <asm/arch/dmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include "setup.h"

/* TODO(clchiou): Sort out setup.h to use DDR3_* macros directly */

#define RDLVL_RDDATA_ADJ	DDR3_RDLVL_RDDATA_ADJ

#define DMC_MEMCONTROL_VAL	DDR3_DMC_MEMCONTROL_VAL
#define DMC_MEMCONFIG_VAL	DDR3_DMC_MEMCONFIG_VAL

#define DMC_TIMINGREF_VAL	DDR3_DMC_TIMINGREF_VAL
#define DMC_TIMINGROW_VAL	DDR3_DMC_TIMINGROW_VAL
#define DMC_TIMINGDATA_VAL	DDR3_DMC_TIMINGDATA_VAL
#define DMC_TIMINGPOWER_VAL	DDR3_DMC_TIMINGPOWER_VAL

#define CTRL_BSTLEN		DDR3_CTRL_BSTLEN
#define CTRL_RDLAT		DDR3_CTRL_RDLAT

#define RD_FETCH		DDR3_RD_FETCH

/*
 * APLL		: 1GHz
 * MCLK_CDREX	: 667Mhz
 * Memory Type	: DDR3
 */

static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	writel(LPDDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	sdelay(10000);	/* approx 200us */
	writel(LPDDR3PHY_CTRL_PHY_RESET, &clk->lpddr3phy_ctrl);
}

/* Sending ZQINIT command */
static void config_zqinit(struct exynos5_dmc *dmc)
{
	unsigned long channel, chip, mask = 0, val;

	for (channel = 0; channel < CONFIG_DMC_CHANNELS; channel++) {
		SET_CMD_CHANNEL(mask, channel);
		for (chip = 0; chip < CONFIG_CHIPS_PER_CHANNEL; chip++) {
			SET_CMD_CHIP(mask, chip);
			val = DIRECT_CMD_ZQINIT | mask;
			writel(val, &dmc->directcmd);
		}
	}
}

static void config_ctrl_dll_on(unsigned int state,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned int val, tmp;

	val = readl(&phy0_ctrl->phy_con13);

	/* Reading ctrl_lock_value[8:2] */
	val &= (NR_DELAY_CELL_COARSE_LOCK_MASK <<
		NR_DELAY_CELL_COARSE_LOCK_OFFSET);

	/* Aligning 'val' to match 'ctrl_force' offset of PHY_CON12 */
	val >>= CTRL_CLOCK_OFFSET;

	/* Setting the PHY_CON12 register */
	tmp = PHY_CON12_RESET_VAL;
	CONFIG_CTRL_DLL_ON(tmp, state);

	/* Writing 'val' in the 'ctrl_force' offset of PHY_CON12 */
	tmp |= val;
	writel(tmp, &phy0_ctrl->phy_con12);

	val = readl(&phy1_ctrl->phy_con13);

	/* Reading ctrl_lock_value[8:2] */
	val &= (NR_DELAY_CELL_COARSE_LOCK_MASK <<
		NR_DELAY_CELL_COARSE_LOCK_OFFSET);

	/* Aligning 'val' to match 'ctrl_force' offset of PHY_CON12 */
	val >>= CTRL_CLOCK_OFFSET;

	/* Setting the PHY_CON12 register */
	tmp = PHY_CON12_RESET_VAL;
	CONFIG_CTRL_DLL_ON(tmp, state);

	/* Writing 'val' in the 'ctrl_force' offset of PHY_CON12 */
	tmp |= val;
	writel(tmp, &phy1_ctrl->phy_con12);
}

void mem_ctrl_init()
{
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	unsigned int val;

	phy0_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY0_BASE;
	phy1_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY1_BASE;
	dmc = (struct exynos5_dmc *)EXYNOS5_DMC_CTRL_BASE;

	reset_phy_ctrl();

	mem_clk_setup();

	/* Setting Operation Mode as DDR3 and enabling byte_rdlvl */
	val = PHY_CON0_RESET_VAL;
	SET_CTRL_DDR_MODE(val, DDR_MODE_DDR3);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* Enable Read Leveling */
	writel(SET_RDLVL_RDDATA_ADJ, &phy0_ctrl->phy_con1);
	writel(SET_RDLVL_RDDATA_ADJ, &phy1_ctrl->phy_con1);

	/* DQS, DQ: Signal */
	val = CTRL_PULLD_DQ | CTRL_PULLD_DQS;
	writel(val, &phy0_ctrl->phy_con14);
	writel(val, &phy1_ctrl->phy_con14);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	writel(PHY_CON42_VAL, &phy0_ctrl->phy_con42);
	writel(PHY_CON42_VAL, &phy1_ctrl->phy_con42);

	/* ZQ Calibration */
	config_zq(phy0_ctrl, phy1_ctrl);

	/*
	 * Set DMC Concontrol
	 * dfi_init_start = 1
	 * rd_fetch = 0x3
	 * empty = 0
	 */
	val = DMC_CONCONTROL_RESET_VAL;
	SET_RD_FETCH(val);
	val |= DFI_INIT_START;
	val &= ~EMPTY;
	writel(val, &dmc->concontrol);

	/*
	 * Dynamic Clock: Always Running
	 * Memory Burst length: 8
	 * Number of chips: 1
	 * Memory Bus width: 32 bit
	 * Memory Type: DDR3
	 * Additional Latancy for PLL: 0 Cycle
	 */
	writel(DMC_MEMCONTROL_VAL, &dmc->memcontrol);

	config_memory(dmc);

	/* Precharge Configuration */
	writel(DMC_PRECHCONFIG_VAL, &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(DMC_PWRDNCONFIG_VAL, &dmc->pwrdnconfig);

	/* Periodic Refresh Interval */
	writel(DMC_TIMINGREF_VAL, &dmc->timingref);

	/*
	 * TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	writel(DMC_TIMINGROW_VAL, &dmc->timingrow);

	writel(DMC_TIMINGDATA_VAL, &dmc->timingdata);

	writel(DMC_TIMINGPOWER_VAL, &dmc->timingpower);

	/* Memory Channel Inteleaving Size: 128 Bytes */
	writel(CONFIG_IV_SIZE, &dmc->ivcontrol);

	/* Set DQS offsets */
	writel(DDR3_PHY0_DQS, &phy0_ctrl->phy_con4);
	writel(DDR3_PHY1_DQS, &phy1_ctrl->phy_con4);

	/* Set DQS offsets */
	writel(DDR3_PHY0_DQ, &phy0_ctrl->phy_con6);
	writel(DDR3_PHY1_DQ, &phy1_ctrl->phy_con6);

	/* Set Debug offsets */
	writel(RESET_DEBUG_OFFSET_VAL, &phy0_ctrl->phy_con10);
	writel(RESET_DEBUG_OFFSET_VAL, &phy1_ctrl->phy_con10);

	config_ctrl_dll_on(RESET, phy0_ctrl, phy1_ctrl);

	sdelay(50);	/* approx 10 PCLK cycle */

	update_reset_dll(dmc);

	config_mrs(dmc);
	config_zqinit(dmc);

	sdelay(50);	/* approx 10 PCLK cycle */

	/* Reset DLL Locking */
	val = PHY_CON12_RESET_VAL;
	CONFIG_CTRL_START(val, RESET);
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	/* Reset DLL Locking */
	val = PHY_CON12_RESET_VAL;
	CONFIG_CTRL_START(val, SET);
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	sdelay(50);	/* approx 10 PCLK cycle */

	/*
	 * Looping till SEC SDRAM PHY initialization completes
	 * for both channel-0 and channel-1
	 */
	do {
		val = readl(&dmc->phystatus);
		val &= (DFI_INIT_COMPLETE_CHO | DFI_INIT_COMPLETE_CH1);
	} while (val == 0);

	/*
	 * Set DMC Concontrol
	 * rd_fetch = 0x3
	 * empty = 0
	 * Basically here we are stopping dfi_init_start done above.
	 */
	val = DMC_CONCONTROL_RESET_VAL;
	SET_RD_FETCH(val);
	val &= ~EMPTY;
	writel(val, &dmc->concontrol);

	update_reset_dll(dmc);

	writel(SET_RDLVL_RDDATA_ADJ, &phy0_ctrl->phy_con1);
	writel(SET_RDLVL_RDDATA_ADJ, &phy1_ctrl->phy_con1);

	/* Enable Read Leveling */
	val = PHY_CON2_RESET_VAL | RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con2);
	writel(val, &phy1_ctrl->phy_con2);

	/* Enable data eye training */
	val = RDLVL_CONFIG_RESET_VAL | CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	/*
	 * Looping till Read level completion happens
	 * for both channel-0 and channel-1
	 */
	do {
		val = readl(&dmc->phystatus);
		val &= (RDLVL_COMPLETE_CHO | RDLVL_COMPLETE_CH1);
	} while (val == 0);

	/* Disable data eye training */
	val = RDLVL_CONFIG_RESET_VAL & ~CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	config_ctrl_dll_on(SET, phy0_ctrl, phy1_ctrl);

	update_reset_dll(dmc);

	config_prech(dmc);

	/*
	 * Dynamic Clock: Always Running
	 * Memory Burst length: 8
	 * Number of chips: 1
	 * Memory Bus width: 32 bit
	 * Memory Type: DDR3
	 * Additional Latancy for PLL: 0 Cycle
	 */
	writel(DMC_MEMCONTROL_VAL, &dmc->memcontrol);

	/*
	 * Set DMC Concontrol
	 *  rd_fetch = 0x3
	 *  empty = 0
	 *  Auto refresh counter enable
	 */
	val = DMC_CONCONTROL_RESET_VAL;
	SET_RD_FETCH(val);
	val &= ~EMPTY;
	val |= AREF_EN;
	writel(val, &dmc->concontrol);
}
