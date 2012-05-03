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
#include "clock_init.h"

static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	writel(LPDDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	writel(LPDDR3PHY_CTRL_PHY_RESET, &clk->lpddr3phy_ctrl);
}

void ddr3_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size)
{
	unsigned int val;
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	phy0_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY0_BASE;
	phy1_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY1_BASE;
	dmc = (struct exynos5_dmc *)EXYNOS5_DMC_CTRL_BASE;

	mem_clk_setup(mem);

	reset_phy_ctrl();

	/* Set Impedance Output Driver */
	val = IMP_OUTPUT_DRV_40_OHM << CA_CK_DRVR_DS_OFFSET |
		IMP_OUTPUT_DRV_40_OHM << CA_CKE_DRVR_DS_OFFSET |
		IMP_OUTPUT_DRV_40_OHM << CA_CS_DRVR_DS_OFFSET |
		IMP_OUTPUT_DRV_40_OHM << CA_ADR_DRVR_DS_OFFSET;
	writel(val, &phy0_ctrl->phy_con39);
	writel(val, &phy1_ctrl->phy_con39);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
		(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	/* ZQ Calibration */
	dmc_config_zq(mem, phy0_ctrl, phy1_ctrl);

	/* DQ Signal */
	writel(mem->phy0_pulld_dqs, &phy0_ctrl->phy_con14);
	writel(mem->phy1_pulld_dqs, &phy1_ctrl->phy_con14);

	writel(mem->concontrol | mem->rd_fetch
		| mem->dfi_init_start, &dmc->concontrol);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	/* DQS Signal */
	writel(mem->phy0_dqs, &phy0_ctrl->phy_con4);
	writel(mem->phy1_dqs, &phy1_ctrl->phy_con4);

	writel(mem->phy0_dq, &phy0_ctrl->phy_con6);
	writel(mem->phy1_dq, &phy1_ctrl->phy_con6);

	writel(mem->phy0_tFS, &phy0_ctrl->phy_con10);
	writel(mem->phy1_tFS, &phy1_ctrl->phy_con10);

	val = mem->ctrl_start_point |
		mem->ctrl_inc |
		mem->ctrl_ddl_on |
		mem->ctrl_ref;
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	/* Start DLL locking */
	writel(val | mem->ctrl_start, &phy0_ctrl->phy_con12);
	writel(val | mem->ctrl_start, &phy1_ctrl->phy_con12);

	update_reset_dll(dmc, DDR_MODE_DDR3);

	writel(mem->concontrol | mem->rd_fetch, &dmc->concontrol);

	/* Memory Channel Inteleaving Size */
	writel(mem->iv_size, &dmc->ivcontrol);

	writel(mem->memconfig, &dmc->memconfig0);
	writel(mem->memconfig, &dmc->memconfig1);
	writel(mem->membaseconfig0, &dmc->membaseconfig0);
	writel(mem->membaseconfig1, &dmc->membaseconfig1);

	/* Precharge Configuration */
	writel(mem->prechconfig, &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(mem->pwrdnconfig, &dmc->pwrdnconfig);

	/* TimingRow, TimingData, TimingPower and Timingaref
	 * values as per Memory AC parameters
	 */
	writel(mem->timing_ref, &dmc->timingref);
	writel(mem->timing_row, &dmc->timingrow);
	writel(mem->timing_data, &dmc->timingdata);
	writel(mem->timing_power, &dmc->timingpower);

	/* Send PALL command */
	dmc_config_prech(mem, dmc);

	/* Send NOP, MRS and ZQINIT commands */
	dmc_config_mrs(mem, dmc);

	writel(mem->memcontrol, &dmc->memcontrol);

	/* Set DMC Concontrol and enable auto-refresh counter */
	writel(mem->concontrol | mem->rd_fetch
		| mem->aref_en, &dmc->concontrol);
}
