/*
 * Clock setup for SMDK5250 board based on EXYNOS5
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
#include <version.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch-exynos/spl.h>

#include "clock_init.h"
#include "setup.h"


struct mem_timings mem_timings[] = {
	{
		.mem_type = DDR_MODE_DDR3,
		.frequency_mhz = 667,
		.bpll_mdiv = 0x185,
		.bpll_pdiv = 0x7,
		.bpll_sdiv = 0x1,
		.pclk_cdrex_ratio = 0x4,
		.direct_cmd_msr = {
			0x00020010, 0x00030000, 0x00010042, 0x00000b50
		},
		.timing_ref = 0x000000bb,
		.timing_row = 0x7645644d,
		.timing_data = 0x45414709,
		.timing_power = 0x3a000a3c,
		.phy0_dqs = 0x08080808,
		.phy1_dqs = 0x08080808,
		.phy0_dq = 0x08080808,
		.phy1_dq = 0x00080808,
		.ctrl_force = 0x33,
		.ctrl_rdlat = 0x09,
		.ctrl_bstlen = 0x08,
		.rd_fetch = 0x2,
		.zq_mode_dds = 7,
		.zq_mode_term = 2,
		.zq_mode_noterm = 0,	/* do nothing for ddr3 */
		.rdlvl_rddata_adj = 0xff00,
		.t_wrrdcmd = 2,
		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 8
		* Number of chips: 1
		* Memory Bus width: 32 bit
		* Memory Type: DDR3
		* Additional Latancy for PLL: 0 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_DISABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_DDR3 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_1 |
			DMC_MEMCONTROL_BL_8 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIGx_CHIP_MAP_INTERLEAVED |
			DMC_MEMCONFIGx_CHIP_COL_10 |
			DMC_MEMCONFIGx_CHIP_ROW_15 |
			DMC_MEMCONFIGx_CHIP_BANK_8,
	}, {
		.mem_type = DDR_MODE_DDR3,
		.frequency_mhz = 800,
		.bpll_mdiv = 0x64,
		.bpll_pdiv = 0x3,
		.bpll_sdiv = 0x0,
		.pclk_cdrex_ratio = 0x5,
		.direct_cmd_msr = {
			0x00020018, 0x00030000, 0x00010042, 0x00000d70
		},
		.timing_ref = 0x000000bb,
		.timing_row = 0x8d46650f,
		.timing_data = 0x4740180b,
		.timing_power = 0x41000a44,
		.phy0_dqs = 0x0f0f0f0f,
		.phy1_dqs = 0x1d1f1f19,
		.phy0_dq = 0x08080808,
		.phy1_dq = 0x88888888,
		.ctrl_force = 0x2a,
		.ctrl_rdlat = 0x0b,
		.ctrl_bstlen = 0x08,
		.rd_fetch = 0x3,
		.zq_mode_dds = 7,
		.zq_mode_term = 2,
		.zq_mode_noterm = 0,	/* do nothing for ddr3 */
		.rdlvl_rddata_adj = 0xff00,
		.t_wrrdcmd = 2,
		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 8
		* Number of chips: 1
		* Memory Bus width: 32 bit
		* Memory Type: DDR3
		* Additional Latancy for PLL: 0 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_DISABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_DDR3 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_1 |
			DMC_MEMCONTROL_BL_8 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIGx_CHIP_MAP_INTERLEAVED |
			DMC_MEMCONFIGx_CHIP_COL_10 |
			DMC_MEMCONFIGx_CHIP_ROW_15 |
			DMC_MEMCONFIGx_CHIP_BANK_8,
	}, {
		.mem_type = DDR_MODE_LPDDR2,
		.frequency_mhz = 667,
		.bpll_mdiv = 0x215,
		.bpll_pdiv = 0xc,
		.bpll_sdiv = 0x1,
		.pclk_cdrex_ratio = 0x3,
		.direct_cmd_msr = {
			0x00071c00, 0x00010bfc, 0x00000708, 0x00000818
		},
		.timing_ref = 0x0000005d,
		.timing_row = 0x2336544C,
		.timing_data = 0x24202408,
		.timing_power = 0x38260235,
		/* ctrl_force seems to be hard-coded in dmc_init_lpddr2.c */
		.ctrl_rdlat = 0x08,
		.ctrl_bstlen = 0x04,
		.rd_fetch = 0x2,
		.zq_mode_dds = 5,
		.zq_mode_term = 5,
		.zq_mode_noterm = 1,
		.rdlvl_rddata_adj = 0x0001,
		/*
		* Dynamic Clock: Always Running
		* Memory Burst length: 4
		* Number of chips: 2
		* Memory Bus width: 32 bit
		* Memory Type: LPDDR2-S4
		* Additional Latancy for PLL: 1 Cycle
		*/
		.memcontrol = DMC_MEMCONTROL_CLK_STOP_DISABLE |
			DMC_MEMCONTROL_DPWRDN_DISABLE |
			DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE |
			DMC_MEMCONTROL_TP_DISABLE |
			DMC_MEMCONTROL_DSREF_DISABLE |
			DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(0) |
			DMC_MEMCONTROL_MEM_TYPE_LPDDR2 |
			DMC_MEMCONTROL_MEM_WIDTH_32BIT |
			DMC_MEMCONTROL_NUM_CHIP_2 |
			DMC_MEMCONTROL_BL_4 |
			DMC_MEMCONTROL_PZQ_DISABLE |
			DMC_MEMCONTROL_MRR_BYTE_7_0,
		.memconfig = DMC_MEMCONFIGx_CHIP_MAP_INTERLEAVED |
			DMC_MEMCONFIGx_CHIP_COL_10 |
			DMC_MEMCONFIGx_CHIP_ROW_14 |
			DMC_MEMCONFIGx_CHIP_BANK_8,
	}
};

#ifdef CONFIG_SPL_BUILD

/**
 * Get the required memory type and speed (SPL version).
 *
 * In SPL we have no device tree, so we use the machine parameters
 *
 * @param mem_type	Returns memory type
 * @param frequency_mhz	Returns memory speed in MHz
 * @return 0 if all ok (if not, this function currently does not return)
 */
static int clock_get_mem_selection(enum ddr_mode *mem_type,
				   unsigned *frequency_mhz)
{
	struct spl_machine_param *params;

	params = spl_get_machine_params();
	*mem_type = params->mem_type;
#ifdef CONFIG_LPDDR2
	*frequency_mhz = 667;
#else
# ifdef CDREX_800
	*frequency_mhz = 800;
# else
	*frequency_mhz = 667;
# endif
#endif
	return 0;
}

#else

/**
 * Get the required memory type and speed (Main U-Boot version).
 *
 * This should use the device tree. For now we cannot since this function is
 * called before the FDT is available.
 *
 * TODO(sjg): Make this function look up the FDT for these parameters.
 *
 * @param mem_type	Returns memory type
 * @param frequency_mhz	Returns memory speed in MHz
 * @return 0 if all ok (if not, this function currently does not return)
 */
static int clock_get_mem_selection(enum ddr_mode *mem_type,
				   unsigned *frequency_mhz)
{
	/* TODO(sjg): Use the device tree values for these */
#ifdef CONFIG_LPDDR2
	*mem_type = DDR_MODE_LPDDR2;
	*frequency_mhz = 667;
#else
	*mem_type = DDR_MODE_DDR3;
# ifdef CDREX_800
	*frequency_mhz = 800;
# else
	*frequency_mhz = 667;
# endif
#endif
	return 0;
}

#endif

struct mem_timings *clock_get_mem_timings(void)
{
	struct mem_timings *mem;
	enum ddr_mode mem_type;
	unsigned frequency_mhz;
	int i;

	if (!clock_get_mem_selection(&mem_type, &frequency_mhz)) {
		for (i = 0, mem = mem_timings; i < ARRAY_SIZE(mem_timings);
				i++, mem++) {
			if (mem->mem_type == mem_type &&
					mem->frequency_mhz == frequency_mhz)
				return mem;
		}
	}
	/* TODO: Call panic() here */
	while (1)
		;
	return NULL;
}

void system_clock_init()
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;
	struct mem_timings *mem;
	u32 val;

	mem = clock_get_mem_timings();

	/*
	 * MUX_APLL_SEL[0]: FINPLL = 0
	 * MUX_CPU_SEL[6]: MOUTAPLL = 0
	 * MUX_HPM_SEL[20]: MOUTAPLL = 0
	 */
	writel(0x0, &clk->src_cpu);

	/* MUX_MPLL_SEL[8]: FINPLL = 0 */
	writel(0x0, &clk->src_core1);

	/*
	 * VPLLSRC_SEL[0]: FINPLL = 0
	 * MUX_{CPLL[8]}|{EPLL[12]}|{VPLL[16]}_SEL: FINPLL = 0
	 */
	writel(0x0, &clk->src_top2);

	/* MUX_BPLL_SEL[0]: FINPLL = 0 */
	writel(0x0, &clk->src_cdrex);

	/* MUX_ACLK_* Clock Selection */
	writel(CLK_SRC_TOP0_VAL, &clk->src_top0);

	/* MUX_ACLK_* Clock Selection */
	writel(CLK_SRC_TOP1_VAL, &clk->src_top1);

	/* MUX_ACLK_* Clock Selection */
	writel(CLK_SRC_TOP3_VAL, &clk->src_top3);

	/* MUX_PWI_SEL[19:16]: SCLKMPLL = 6 */
	writel(CLK_SRC_CORE0_VAL, &clk->src_core0);

	/* MUX_ATCLK_LEX[0]: ACLK_200 = 0 */
	writel(CLK_SRC_LEX_VAL, &clk->src_lex);

	/* UART [0-5]: SCLKMPLL = 6 */
	writel(CLK_SRC_PERIC0_VAL, &clk->src_peric0);

	/* SPI [0-2]: SCLKMPLL = 6 */
	writel(CLK_SRC_PERIC1_VAL, &clk->src_peric1);

	/* ISP SPI [0-1]: SCLKMPLL = 6 */
	writel(SCLK_SRC_ISP_VAL, &clk->sclk_src_isp);

	/* Set Clock Ratios */
	writel(CLK_DIV_CPU0_VAL, &clk->div_cpu0);

	/* Set COPY and HPM Ratio */
	writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);

	/* CORED_RATIO, COREP_RATIO */
	writel(CLK_DIV_CORE0_VAL, &clk->div_core0);

	/* PWI_RATIO[11:8], DVSEM_RATIO[22:16], DPM_RATIO[24:20] */
	writel(CLK_DIV_CORE1_VAL, &clk->div_core1);

	/* ACLK_*_RATIO */
	writel(CLK_DIV_TOP0_VAL, &clk->div_top0);

	/* ACLK_*_RATIO */
	writel(CLK_DIV_TOP1_VAL, &clk->div_top1);

	/* CDREX Ratio */
	writel(CLK_DIV_CDREX_INIT_VAL, &clk->div_cdrex);

	/* MCLK_EFPHY_RATIO[3:0] */
	writel(CLK_DIV_CDREX2_VAL, &clk->div_cdrex2);

	/* {PCLK[4:6]|ATCLK[10:8]}_RATIO */
	writel(CLK_DIV_LEX_VAL, &clk->div_lex);

	/* PCLK_R0X_RATIO[3:0] */
	writel(CLK_DIV_R0X_VAL, &clk->div_r0x);

	/* PCLK_R1X_RATIO[3:0] */
	writel(CLK_DIV_R1X_VAL, &clk->div_r1x);

	/* SATA[24]: SCLKMPLL=0, MMC[0-4]: SCLKMPLL = 6 */
	writel(CLK_SRC_FSYS_VAL, &clk->src_fsys);

	/* UART[0-4] */
	writel(CLK_DIV_PERIC0_VAL, &clk->div_peric0);

	/* SPI[0-1] */
	writel(CLK_DIV_PERIC1_VAL, &clk->div_peric1);

	/* SPI[2] */
	writel(CLK_DIV_PERIC2_VAL, &clk->div_peric2);

	/* ISP SPI[0-1] */
	writel(SCLK_DIV_ISP_VAL, &clk->sclk_div_isp);

	/* PWM_RATIO[3:0] */
	writel(CLK_DIV_PERIC3_VAL, &clk->div_peric3);

	/* SATA_RATIO, USB_DRD_RATIO */
	writel(CLK_DIV_FSYS0_VAL, &clk->div_fsys0);

	/* MMC[0-1] */
	writel(CLK_DIV_FSYS1_VAL, &clk->div_fsys1);

	/* MMC[2-3] */
	writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);

	/* MMC[4] */
	writel(CLK_DIV_FSYS3_VAL, &clk->div_fsys3);

	/* ACLK|PLCK_ACP_RATIO */
	writel(CLK_DIV_ACP_VAL, &clk->div_acp);

	/* ISPDIV0_RATIO, ISPDIV1_RATIO */
	writel(CLK_DIV_ISP0_VAL, &clk->div_isp0);

	/* MCUISPDIV0_RATIO, MCUISPDIV1_RATIO */
	writel(CLK_DIV_ISP1_VAL, &clk->div_isp1);

	/* MPWMDIV_RATIO */
	writel(CLK_DIV_ISP2_VAL, &clk->div_isp2);

	/* FIMD1 SRC CLK SELECTION */
	writel(CLK_SRC_DISP1_0_VAL, &clk->src_disp1_0);

	/* PLL locktime */
	writel(APLL_LOCK_VAL, &clk->apll_lock);

	writel(MPLL_LOCK_VAL, &clk->mpll_lock);

	writel(BPLL_LOCK_VAL, &clk->bpll_lock);

	writel(CPLL_LOCK_VAL, &clk->cpll_lock);

	writel(EPLL_LOCK_VAL, &clk->epll_lock);

	writel(VPLL_LOCK_VAL, &clk->vpll_lock);

	sdelay(0x10000);

	/* Set APLL */
	writel(APLL_CON1_VAL, &clk->apll_con1);
	writel(APLL_CON0_VAL, &clk->apll_con0);
	sdelay(0x30000);

	/* Set MPLL */
	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	writel(MPLL_CON0_VAL, &clk->mpll_con0);
	sdelay(0x30000);
	writel(BPLL_CON1_VAL, &clk->bpll_con1);
	val = set_pll(mem->bpll_mdiv, mem->bpll_pdiv, mem->bpll_sdiv);
	writel(val, &clk->bpll_con0);
	sdelay(0x30000);

	/* Set CPLL */
	writel(CPLL_CON1_VAL, &clk->cpll_con1);
	writel(CPLL_CON0_VAL, &clk->cpll_con0);
	sdelay(0x30000);

	/* Set EPLL */
	writel(EPLL_CON2_VAL, &clk->epll_con2);
	writel(EPLL_CON1_VAL, &clk->epll_con1);
	writel(EPLL_CON0_VAL, &clk->epll_con0);
	sdelay(0x30000);

	/* Set VPLL */
	writel(VPLL_CON2_VAL, &clk->vpll_con2);
	writel(VPLL_CON1_VAL, &clk->vpll_con1);
	writel(VPLL_CON0_VAL, &clk->vpll_con0);
	sdelay(0x30000);

	/* Set MPLL */
	/* After Initiallising th PLL select the sources accordingly */
	/* MUX_APLL_SEL[0]: MOUTAPLLFOUT = 1 */
	writel(CLK_SRC_CPU_VAL, &clk->src_cpu);

	/* MUX_MPLL_SEL[8]: MOUTMPLLFOUT = 1 */
	writel(CLK_SRC_CORE1_VAL, &clk->src_core1);

	/* MUX_BPLL_SEL[0]: FOUTBPLL = 1*/
	writel(CLK_SRC_CDREX_INIT_VAL, &clk->src_cdrex);

	/*
	 * VPLLSRC_SEL[0]: FINPLL = 0
	 * MUX_{CPLL[8]}|{EPLL[12]}|{VPLL[16]}_SEL: MOUT{CPLL|EPLL|VPLL} = 1
	 * MUX_{MPLL[20]}|{BPLL[24]}_USER_SEL: FOUT{MPLL|BPLL} = 1
	 */
	writel(CLK_SRC_TOP2_VAL, &clk->src_top2);
}

void mem_clk_setup(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;
	struct mem_timings *mem;
	u32 val;

	mem = clock_get_mem_timings();

	writel(0x0, &clk->src_cdrex);
	val = (MCLK_CDREX2_RATIO << 28)
		| (ACLK_EFCON_RATIO << 24)
		| (MCLK_DPHY_RATIO << 20)
		| (MCLK_CDREX_RATIO << 16)
		| (ACLK_C2C_200_RATIO << 12)
		| (C2C_CLK_400_RATIO << 8)
		| (mem->pclk_cdrex_ratio << 4)
		| (ACLK_CDREX_RATIO);
	writel(val, &clk->div_cdrex);

	writel(MPLL_CON1_VAL, &clk->mpll_con1);
	writel(MPLL_CON0_VAL, &clk->mpll_con0);
	writel(BPLL_CON1_VAL, &clk->bpll_con1);
	val = set_pll(mem->bpll_mdiv, mem->bpll_pdiv, mem->bpll_sdiv);
	writel(val, &clk->bpll_con0);

	writel(CLK_SRC_CDREX_VAL, &clk->src_cdrex);
}

#ifdef CONFIG_SPL_BUILD
/*
 * This is a custom implementation for the udelay(), as we do not the timer
 * initialise during the SPL boot. We are assuming the cpu takes 3 instruction
 * pre cycle. This is based on the implementation of sdelay() function.
 */
void udelay(unsigned long usec)
{
	unsigned long count;

	/* TODO(alim.akhtar@samsung.com): Comment on why divided by 30000000 */
	count = usec * (get_pll_clk(APLL) / (3 * 10000000));
	sdelay(count);
}
#endif
