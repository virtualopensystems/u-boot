/*
 * LPDDR2 mem setup file for SMDK5250 board based on EXYNOS5
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
#include "clock_init.h"
#include "setup.h"

#define SET_CTRL_FORCE_VAL(x, y)	(x = (x & ~(0x7F << 8)) | y << 8)


static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	writel(LPDDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	sdelay(0x10000);
}

static void sec_sdram_phy_init(struct exynos5_dmc *dmc)
{
	unsigned long val;
	val = readl(&dmc->concontrol);
	val |= DFI_INIT_START;
	writel(val, &dmc->concontrol);
	sdelay(0x10000);

	val = readl(&dmc->concontrol);
	val &= ~DFI_INIT_START;
	writel(val, &dmc->concontrol);
}

static void config_offsets(unsigned int state,
				struct exynos5_phy_control *phy0_ctrl,
				struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	/* Set Offsets to read DQS */
	val = state ? SET_DQS_OFFSET_VAL : RESET_DQS_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con4);
	writel(val, &phy1_ctrl->phy_con4);

	/* Set Offsets to read DQ */
	val = state ? SET_DQ_OFFSET_VAL : RESET_DQ_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con6);
	writel(val, &phy1_ctrl->phy_con6);

	/* Debug Offset */
	val = state ? SET_DEBUG_OFFSET_VAL : RESET_DEBUG_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con10);
	writel(val, &phy1_ctrl->phy_con10);
}

static void config_cdrex(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;
	struct mem_timings *mem;
	u32 val;

	mem = clock_get_mem_timings();
	val = (MCLK_CDREX2_RATIO << 28)
		| (ACLK_EFCON_RATIO << 24)
		| (MCLK_DPHY_RATIO << 20)
		| (MCLK_CDREX_RATIO << 16)
		| (ACLK_C2C_200_RATIO << 12)
		| (C2C_CLK_400_RATIO << 8)
		| (mem->pclk_cdrex_ratio << 4)
		| (ACLK_CDREX_RATIO);
	writel(val, &clk->div_cdrex);
	writel(CLK_SRC_CDREX_VAL, &clk->src_cdrex);
	sdelay(0x30000);
}

/**
 * Turn the DLL on or off
 *
 * @param state			0 to turn off, 1 to turn on
 * @param ctrl_force_val	Value for the ctrl_force field
 * @param phy0_ctrl		Pointer to PHY0 control registers
 * @param phy1_ctrl		Pointer to PHY1 control registers
 */
static void config_ctrl_dll_on(unsigned int state,
			unsigned int ctrl_force_val,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	val = readl(&phy0_ctrl->phy_con12);
	CONFIG_CTRL_DLL_ON(val, state);
	SET_CTRL_FORCE_VAL(val, ctrl_force_val);
	writel(val, &phy0_ctrl->phy_con12);

	val = readl(&phy1_ctrl->phy_con12);
	CONFIG_CTRL_DLL_ON(val, state);
	SET_CTRL_FORCE_VAL(val, ctrl_force_val);
	writel(val, &phy1_ctrl->phy_con12);
}

/**
 * Start/stop DLL locking
 *
 * @param state			0 to stop DLL locking, 1 to start
 * @param phy0_ctrl		Pointer to PHY0 control registers
 * @param phy1_ctrl		Pointer to PHY1 control registers
 */
static void config_ctrl_start(unsigned int state,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	val = readl(&phy0_ctrl->phy_con12);
	CONFIG_CTRL_START(val, state);
	writel(val, &phy0_ctrl->phy_con12);

	val = readl(&phy1_ctrl->phy_con12);
	CONFIG_CTRL_START(val, state);
	writel(val, &phy1_ctrl->phy_con12);
}

#if defined(CONFIG_RD_LVL)
static void config_rdlvl(struct mem_timings *mem, struct exynos5_dmc *dmc,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;

	/* Disable CTRL_DLL_ON and set ctrl_force */
	config_ctrl_dll_on(0, 0x2D, phy0_ctrl, phy1_ctrl);

	/*
	 * Set ctrl_gateadj, ctrl_readadj
	 * ctrl_gateduradj, rdlvl_pass_adj (all to 0)
	 * rdlvl_rddata_adj
	 */
	val = PHY_CON1_RESET_VAL;
	val |= mem->rdlvl_rddata_adj << PHY_CON1_RDLVL_RDDATA_ADJ_SHIFT;
	writel(val, &phy0_ctrl->phy_con1);
	writel(val, &phy1_ctrl->phy_con1);

	/* LPDDR2 Address */
	writel(LPDDR2_ADDR, &phy0_ctrl->phy_con22);
	writel(LPDDR2_ADDR, &phy1_ctrl->phy_con22);

	/* Enable Byte Read Leveling set ctrl_ddr_mode */
	val = readl(&phy0_ctrl->phy_con0);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	val = readl(&phy1_ctrl->phy_con0);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy1_ctrl->phy_con0);

	/* rdlvl_en: Use levelling offset instead ctrl_shiftc */
	val = PHY_CON2_RESET_VAL | RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con2);
	writel(val, &phy1_ctrl->phy_con2);
	sdelay(0x10000);

	/* Enable Data Eye Training */
	val = readl(&dmc->rdlvl_config);
	val |= CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);
	sdelay(0x10000);

	/* Disable Data Eye Training */
	val = readl(&dmc->rdlvl_config);
	val &= ~CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	/* RdDeSkew_clear: Clear */
	val = readl(&phy0_ctrl->phy_con2);
	val |= RDDSKEW_CLEAR;
	writel(val, &phy0_ctrl->phy_con2);
	val = readl(&phy1_ctrl->phy_con2);
	val |= RDDSKEW_CLEAR;
	writel(val, &phy1_ctrl->phy_con2);

	/* Enable CTRL_DLL_ON */
	config_ctrl_dll_on(1, 0x0, phy0_ctrl, phy1_ctrl);

	update_reset_dll(dmc, DDR_MODE_LPDDR2);
	sdelay(0x10000);

	/* ctrl_atgte: ctrl_gate_p*, ctrl_read_p* generated by PHY */
	val = readl(&phy0_ctrl->phy_con0);
	val &= ~CTRL_ATGATE;
	writel(val, &phy0_ctrl->phy_con0);
	val = readl(&phy1_ctrl->phy_con0);
	val &= ~CTRL_ATGATE;
	writel(val, &phy1_ctrl->phy_con0);
}
#endif

void lpddr2_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size)
{
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	unsigned long val;

	phy0_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY0_BASE;
	phy1_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY1_BASE;
	dmc = (struct exynos5_dmc *)EXYNOS5_DMC_CTRL_BASE;

	/* Reset PHY Controllor: PHY_RESET[0] */
	reset_phy_ctrl();

	/*set Read Latancy and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
			(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	/* ZQ Cofiguration */
	dmc_config_zq(mem, phy0_ctrl, phy1_ctrl);

	/* Operation Mode : LPDDR2 */
	val = PHY_CON0_RESET_VAL;
	SET_CTRL_DDR_MODE(val, DDR_MODE_LPDDR2);
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* DQS, DQ: Signal, for LPDDR2: Always Set */
	val = CTRL_PULLD_DQ | CTRL_PULLD_DQS;
	writel(val, &phy0_ctrl->phy_con14);
	writel(val, &phy1_ctrl->phy_con14);

	/* Init SEC SDRAM PHY */
	sec_sdram_phy_init(dmc);
	sdelay(0x10000);

	update_reset_dll(dmc, DDR_MODE_LPDDR2);

	writel(mem->memcontrol, &dmc->memcontrol);

	dmc_config_memory(mem, dmc);

	/* Precharge Configuration */
	writel(DMC_PRECHCONFIG_VAL, &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(DMC_PWRDNCONFIG_VAL, &dmc->pwrdnconfig);

	/* Periodic Refrese Interval */
	writel(mem->timing_ref, &dmc->timingref);

	/*
	 * TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	writel(mem->timing_row, &dmc->timingrow);
	writel(mem->timing_data, &dmc->timingdata);
	writel(mem->timing_power, &dmc->timingpower);

	/* Memory Channel Inteleaving Size: 128 Bytes */
	writel(mem_iv_size, &dmc->ivcontrol);

	/* Set DQS, DQ and DEBUG offsets */
	config_offsets(1, phy0_ctrl, phy1_ctrl);

	/* Disable CTRL_DLL_ON and set ctrl_force */
	config_ctrl_dll_on(0, 0x7F, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	update_reset_dll(dmc, DDR_MODE_LPDDR2);

	/* Config MRS(Mode Register Settingg) */
	config_mrs(dmc);

	config_cdrex();

	/* Reset DQS DQ and DEBUG offsets */
	config_offsets(0, phy0_ctrl, phy1_ctrl);

	/* Enable CTRL_DLL_ON */
	config_ctrl_dll_on(1, 0x0, phy0_ctrl, phy1_ctrl);

	/* Stop DLL Locking */
	config_ctrl_start(0, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	/* Start DLL Locking */
	config_ctrl_start(1, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	update_reset_dll(dmc, DDR_MODE_LPDDR2);

#if defined(CONFIG_RD_LVL)
	config_rdlvl(mem, dmc, phy0_ctrl, phy1_ctrl);
#endif
	config_prech(dmc);

	/*
	 * Dynamic Clock: Stops During Idle Period
	 * Dynamic Power Down: Enable
	 * Dynamic Self refresh: Enable
	 */
	val = readl(&dmc->memcontrol);
	val |= CLK_STOP_EN | DPWRDN_EN | DSREF_EN;
	writel(val, &dmc->memcontrol);

	/* Start Auto refresh */
	val = readl(&dmc->concontrol);
	val |= AREF_EN;
	writel(val, &dmc->concontrol);
}
