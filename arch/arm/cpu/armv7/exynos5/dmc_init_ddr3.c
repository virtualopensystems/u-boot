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

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/dmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>

#include "clock_init.h"
#include "setup.h"

static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	writel(LPDDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	writel(LPDDR3PHY_CTRL_PHY_RESET, &clk->lpddr3phy_ctrl);
}

static void config_ctrl_dll_on(unsigned int state,
			struct exynos5_phy_control *phy_ctrl)
{
	unsigned int val, tmp;

	val = readl(&phy_ctrl->phy_con13);

	/* Reading ctrl_lock_value[8:2] */
	val &= (NR_DELAY_CELL_COARSE_LOCK_MASK <<
		NR_DELAY_CELL_COARSE_LOCK_OFFSET);

	/* Aligning 'val' to match 'ctrl_force' offset of PHY_CON12 */
	val >>= CTRL_CLOCK_OFFSET;

	/* Setting the PHY_CON12 register */
	tmp = PHY_CON12_RESET_VAL;
	assert(state == 0 || state == 1);
	clrsetbits_le32(&tmp, PHY_CON12_CTRL_DLL_ON_MASK,
			state << PHY_CON12_CTRL_DLL_ON_SHIFT);

	/* Writing 'val' in the 'ctrl_force' offset of PHY_CON12 */
	tmp |= val;
	writel(tmp, &phy_ctrl->phy_con12);
}

void ddr3_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size)
{
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	unsigned int val;
	unsigned int phy_con1;

	phy0_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY0_BASE;
	phy1_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY1_BASE;
	dmc = (struct exynos5_dmc *)EXYNOS5_DMC_CTRL_BASE;

	mem_clk_setup(mem);

	sdelay(40000);

	reset_phy_ctrl();

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
			(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	/* Setting Operation Mode as DDR3 and enabling byte_rdlvl */
	val = PHY_CON0_RESET_VAL;
	val |= mem->mem_type << PHY_CON0_CTRL_DDR_MODE_SHIFT;
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* ZQ Calibration */
	dmc_config_zq(mem, phy0_ctrl, phy1_ctrl);

	/* DQS, DQ: Signal */
	val = CTRL_PULLD_DQ | CTRL_PULLD_DQS;
	writel(val, &phy0_ctrl->phy_con14);
	writel(val, &phy1_ctrl->phy_con14);

	val = PHY_CON12_RESET_VAL;
	val |= mem->ctrl_force << 8;
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	/* Set DMC Concontrol */
	val = DMC_CONCONTROL_RESET_VAL;
	val |= mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT;
	val |= DFI_INIT_START;
	writel(val, &dmc->concontrol);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	/* Set DQS offsets */
	writel(mem->phy0_dqs, &phy0_ctrl->phy_con4);
	writel(mem->phy1_dqs, &phy1_ctrl->phy_con4);

	/* Set DQS offsets */
	writel(mem->phy0_dq, &phy0_ctrl->phy_con6);
	writel(mem->phy1_dq, &phy1_ctrl->phy_con6);

	writel(mem->memcontrol, &dmc->memcontrol);

	dmc_config_memory(mem, dmc);

	/* Memory Channel Inteleaving Size: 128 Bytes */
	writel(mem_iv_size, &dmc->ivcontrol);

	/* Precharge Configuration */
	writel(DMC_PRECHCONFIG_VAL, &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(DMC_PWRDNCONFIG_VAL, &dmc->pwrdnconfig);

	/* Periodic Refresh Interval */
	writel(mem->timing_ref, &dmc->timingref);

	/*
	 * TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	writel(mem->timing_row, &dmc->timingrow);
	writel(mem->timing_data, &dmc->timingdata);
	writel(mem->timing_power, &dmc->timingpower);

	dmc_config_mrs(mem, dmc);

	config_ctrl_dll_on(0, phy0_ctrl);
	config_ctrl_dll_on(0, phy1_ctrl);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	/* Enable Read Leveling */
	phy_con1 = PHY_CON1_RESET_VAL;
	phy_con1 |= mem->rdlvl_rddata_adj << PHY_CON1_RDLVL_RDDATA_ADJ_SHIFT;
	writel(phy_con1, &phy0_ctrl->phy_con1);
	writel(phy_con1, &phy1_ctrl->phy_con1);

	/* Write DDR3 address */
	writel(DDR3_ADDR, &phy0_ctrl->phy_con24);
	writel(DDR3_ADDR, &phy1_ctrl->phy_con24);

	/* Setting Operation Mode as DDR3 and enabling byte_rdlvl */
	val = PHY_CON0_RESET_VAL;
	val |= mem->mem_type << PHY_CON0_CTRL_DDR_MODE_SHIFT;
	clrsetbits_le32(&val, PHY_CON0_T_WRRDCMD_MASK,
		mem->t_wrrdcmd << PHY_CON0_T_WRRDCMD_SHIFT);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* Enable Read Leveling */
	val = PHY_CON2_RESET_VAL | RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con2);
	writel(val, &phy1_ctrl->phy_con2);

	/* Enable data eye training */
	val = RDLVL_CONFIG_RESET_VAL | CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	sdelay(10000);

	/* Disable data eye training */
	val = RDLVL_CONFIG_RESET_VAL & ~CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	config_ctrl_dll_on(1, phy0_ctrl);
	config_ctrl_dll_on(1, phy1_ctrl);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	val = PHY_CON12_RESET_VAL;
	val |= mem->ctrl_force << 8;
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	/* Setting Operation Mode as DDR3 and enabling byte_rdlvl */
	val = PHY_CON0_RESET_VAL;
	val |= mem->mem_type << PHY_CON0_CTRL_DDR_MODE_SHIFT;
	clrsetbits_le32(&val, PHY_CON0_T_WRRDCMD_MASK,
		mem->t_wrrdcmd << PHY_CON0_T_WRRDCMD_SHIFT);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* Enable Read Leveling */
	writel(phy_con1, &phy0_ctrl->phy_con1);
	writel(phy_con1, &phy1_ctrl->phy_con1);

	writel(mem->memcontrol, &dmc->memcontrol);

	/*
	 * Set DMC Concontrol
	 * Auto refresh counter enable
	 */
	val = DMC_CONCONTROL_RESET_VAL;
	val |= mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT;
	val |= DFI_INIT_START;
	val |= AREF_EN;
	writel(val, &dmc->concontrol);
}
