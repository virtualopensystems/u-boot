/*
 * Mem setup common file for different types of DDR present on SMDK5250 boards.
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

#include "setup.h"
#include <asm/arch-exynos/spl.h>

/* TODO(clchiou): Move to device tree */
#ifdef CONFIG_LPDDR2
#define ZQ_MODE_DDS_VAL		LPDDR2_ZQ_MODE_DDS_VAL
#define ZQ_MODE_TERM_VAL	LPDDR2_ZQ_MODE_TERM_VAL
#define ZQ_MODE_NOTERM		LPDDR2_ZQ_MODE_NOTERM
#define DIRECT_CMD_MRS1		LPDDR2_DIRECT_CMD_MRS1
#define DIRECT_CMD_MRS2		LPDDR2_DIRECT_CMD_MRS2
#define DIRECT_CMD_MRS3		LPDDR2_DIRECT_CMD_MRS3
#define DIRECT_CMD_MRS4		LPDDR2_DIRECT_CMD_MRS4
#define DMC_MEMCONTROL_VAL	LPDDR2_DMC_MEMCONTROL_VAL
#define DMC_MEMCONFIG_VAL	LPDDR2_DMC_MEMCONFIG_VAL
#elif defined CONFIG_DDR3
#define ZQ_MODE_DDS_VAL		DDR3_ZQ_MODE_DDS_VAL
#define ZQ_MODE_TERM_VAL	DDR3_ZQ_MODE_TERM_VAL
#define ZQ_MODE_NOTERM		DDR3_ZQ_MODE_NOTERM
#define DIRECT_CMD_MRS1		DDR3_DIRECT_CMD_MRS1
#define DIRECT_CMD_MRS2		DDR3_DIRECT_CMD_MRS2
#define DIRECT_CMD_MRS3		DDR3_DIRECT_CMD_MRS3
#define DIRECT_CMD_MRS4		DDR3_DIRECT_CMD_MRS4
#define DMC_MEMCONTROL_VAL	DDR3_DMC_MEMCONTROL_VAL
#define DMC_MEMCONFIG_VAL	DDR3_DMC_MEMCONFIG_VAL
#endif

void config_zq(struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val = 0;

	/*
	 * ZQ Calibration:
	 * Select Driver Strength,
	 * long calibration for manual calibration
	 */
	val = PHY_CON16_RESET_VAL;
	SET_ZQ_MODE_DDS_VAL(val);
	SET_ZQ_MODE_TERM_VAL(val);
	val |= ZQ_CLK_DIV_EN;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* Disable termination */
	val |= ZQ_MODE_NOTERM;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* ZQ_MANUAL_START: Enable */
	val |= ZQ_MANUAL_STR;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);
	sdelay(0x20000);

	/* ZQ_MANUAL_START: Disable */
	val &= ~ZQ_MANUAL_STR;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);
}

void update_reset_dll(struct exynos5_dmc *dmc, enum ddr_mode mode)
{
	unsigned long val;

	if (mode == DDR_MODE_DDR3) {
		val = MEM_TERM_EN | PHY_TERM_EN | CTRL_SHGATE;
		writel(val, &dmc->phycontrol0);
	}

	/* Update DLL Information: Force DLL Resyncronization */
	val = readl(&dmc->phycontrol0);
	val |= FP_RSYNC;
	writel(val, &dmc->phycontrol0);

	/* Reset Force DLL Resyncronization */
	val = readl(&dmc->phycontrol0);
	val &= ~FP_RSYNC;
	writel(val, &dmc->phycontrol0);
}

void config_mrs(struct exynos5_dmc *dmc)
{
	unsigned long channel, chip, mask = 0;

	for (channel = 0; channel < CONFIG_DMC_CHANNELS; channel++) {
		SET_CMD_CHANNEL(mask, channel);
		for (chip = 0; chip < CONFIG_CHIPS_PER_CHANNEL; chip++) {
			SET_CMD_CHIP(mask, chip);

			/* Sending NOP command */
			writel(DIRECT_CMD_NOP | mask, &dmc->directcmd);
			sdelay(0x10000);

			/* Sending EMRS/MRS commands */
			writel(DIRECT_CMD_MRS1 | mask, &dmc->directcmd);
			sdelay(0x10000);

			writel(DIRECT_CMD_MRS2 | mask, &dmc->directcmd);
			sdelay(0x10000);

			writel(DIRECT_CMD_MRS3 | mask, &dmc->directcmd);
			sdelay(0x10000);

			writel(DIRECT_CMD_MRS4 | mask, &dmc->directcmd);
			sdelay(0x10000);
		}
	}
}

void config_prech(struct exynos5_dmc *dmc)
{
	unsigned long channel, chip, mask = 0;

	for (channel = 0; channel < CONFIG_DMC_CHANNELS; channel++) {
		SET_CMD_CHANNEL(mask, channel);
		for (chip = 0; chip < CONFIG_CHIPS_PER_CHANNEL; chip++) {
			SET_CMD_CHIP(mask, chip);
			/* PALL (all banks precharge) CMD */
			writel(DIRECT_CMD_PALL | mask, &dmc->directcmd);
			sdelay(0x10000);
		}
	}
}

void config_memory(struct exynos5_dmc *dmc)
{
	writel(DMC_MEMCONFIG0_VAL, &dmc->memconfig0);
	writel(DMC_MEMCONFIG1_VAL, &dmc->memconfig1);
	writel(DMC_MEMBASECONFIG0_VAL, &dmc->membaseconfig0);
	writel(DMC_MEMBASECONFIG1_VAL, &dmc->membaseconfig1);
}

void mem_ctrl_init()
{
	struct spl_machine_param *param = spl_get_machine_params();

	if (param->mem_type == DDR_MODE_DDR3)
		ddr3_mem_ctrl_init(param->mem_iv_size);
	else
		lpddr2_mem_ctrl_init(param->mem_iv_size);
}
