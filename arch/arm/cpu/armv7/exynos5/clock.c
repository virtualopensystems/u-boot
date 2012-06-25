/*
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>

/* src_bit div_bit prediv_bit */
static struct clk_bit_info clk_bit_info[PERIPH_ID_COUNT] = {
	{0,	0,	-1},
	{4,	4,	-1},
	{8,	8,	-1},
	{12,	12,	-1},
	{0,	0,	8},
	{4,	16,	24},
	{8,	0,	8},
	{12,	16,	24},
	{-1,	-1,	-1},
	{16,	0,	8},
	{20,	16,	24},
	{24,	0,	8},
	{0,	0,	4},
	{4,	12,	16},
	{-1,	-1,	-1},
	{-1,	-1,	-1},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{-1,	24,	0},
	{24,	0,	-1},
	{24,	0,	-1},
	{24,	0,	-1},
	{24,	0,	-1},
	{24,	0,	-1},
};

/* exynos5: return pll clock frequency */
unsigned long get_pll_clk(int pllreg)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long r, m, p, s, k = 0, mask, fout;
	unsigned int freq;

	switch (pllreg) {
	case APLL:
		r = readl(&clk->apll_con0);
		break;
	case MPLL:
		r = readl(&clk->mpll_con0);
		break;
	case EPLL:
		r = readl(&clk->epll_con0);
		k = readl(&clk->epll_con1);
		break;
	case VPLL:
		r = readl(&clk->vpll_con0);
		k = readl(&clk->vpll_con1);
		break;
	default:
		debug("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	/*
	 * APLL_CON: MIDV [25:16]
	 * MPLL_CON: MIDV [25:16]
	 * EPLL_CON: MIDV [24:16]
	 * VPLL_CON: MIDV [24:16]
	 */
	if (pllreg == APLL || pllreg == MPLL)
		mask = 0x3ff;
	else
		mask = 0x1ff;

	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	freq = CONFIG_SYS_CLK_FREQ;

	if (pllreg == EPLL) {
		k = k & 0xffff;
		/* FOUT = (MDIV + K / 65536) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 65536) * (freq / (p * (1 << s)));
	} else if (pllreg == VPLL) {
		k = k & 0xfff;
		/* FOUT = (MDIV + K / 1024) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 1024) * (freq / (p * (1 << s)));
	} else {
		if (s < 1)
			s = 1;
		/* FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) */
		fout = m * (freq / (p * (1 << (s - 1))));

		/* According to the user manual, in EVT1 MPLL always gives
		 * 1.6GHz clock, so divide by 2 to get 800MHz MPLL clock.
		 */
		if (pllreg == MPLL)
			fout /= 2;
	}

	return fout;
}

unsigned long clock_get_periph_rate(enum periph_id peripheral)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	struct clk_bit_info *bit_info = &clk_bit_info[peripheral];
	unsigned long sclk, sub_clk;
	unsigned int src, div, sub_div;

	switch (peripheral) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
		src = readl(&clk->src_peric0);
		div = readl(&clk->div_peric0);
		break;
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
	case PERIPH_ID_PWM4:
		src = readl(&clk->src_peric0);
		div = readl(&clk->div_peric3);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
		src = readl(&clk->src_peric1);
		div = readl(&clk->div_peric1);
		break;
	case PERIPH_ID_SPI2:
		src = readl(&clk->src_peric1);
		div = readl(&clk->div_peric2);
		break;
	case PERIPH_ID_SPI3:
	case PERIPH_ID_SPI4:
		src = readl(&clk->sclk_src_isp);
		div = readl(&clk->sclk_div_isp);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
	case PERIPH_ID_SDMMC2:
	case PERIPH_ID_SDMMC3:
		src = readl(&clk->src_fsys);
		div = readl(&clk->div_fsys1);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
	case PERIPH_ID_I2C6:
	case PERIPH_ID_I2C7:
		sclk = get_pll_clk(MPLL);
		sub_div = ((readl(&clk->div_top1) >> bit_info->div_bit) & 0x7) + 1;
		div = ((readl(&clk->div_top0) >> bit_info->prediv_bit) & 0x7) + 1;
		return (sclk / sub_div) / div;
	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	};

	src = (src >> bit_info->src_bit) & 0xf;
	if (src == SRC_MPLL)
		sclk = get_pll_clk(MPLL);
	else if (src == SRC_EPLL)
		sclk = get_pll_clk(EPLL);
	else if (src == SRC_VPLL)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	sub_div = (div >> bit_info->div_bit) & 0xf;
	sub_clk = sclk / (sub_div + 1);

	if (peripheral == PERIPH_ID_SDMMC0 || peripheral == PERIPH_ID_SDMMC2) {
		div = (div >> bit_info->prediv_bit) & 0xff;
		return sub_clk / (div + 1);
	}

	return sub_clk;
}

/* exynos5: return ARM clock frequency */
unsigned long get_arm_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long div;
	unsigned long armclk;
	unsigned int arm_ratio;
	unsigned int arm2_ratio;

	div = readl(&clk->div_cpu0);

	/* ARM_RATIO: [2:0], ARM2_RATIO: [30:28] */
	arm_ratio = (div >> 0) & 0x7;
	arm2_ratio = (div >> 28) & 0x7;

	armclk = get_pll_clk(APLL) / (arm_ratio + 1);
	armclk /= (arm2_ratio + 1);

	return armclk;
}

/* exynos5: return pwm clock frequency */
unsigned long get_pwm_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long pclk, sclk;
	unsigned int ratio;

	/*
	 * CLK_DIV_PERIC3
	 * PWM_RATIO [3:0]
	 */
	ratio = readl(&clk->div_peric3);
	ratio = ratio & 0xf;
	sclk = get_pll_clk(MPLL);

	pclk = sclk / (ratio + 1);

	return pclk;
}

/* exynos5: return uart clock frequency */
unsigned long get_uart_clk(int dev_index)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long uclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	/*
	 * CLK_SRC_PERIC0
	 * UART0_SEL [3:0]
	 * UART1_SEL [7:4]
	 * UART2_SEL [8:11]
	 * UART3_SEL [12:15]
	 * UART4_SEL [16:19]
	 * UART5_SEL [23:20]
	 */
	sel = readl(&clk->src_peric0);
	sel = (sel >> (dev_index << 2)) & 0xf;

	if (sel == 0x6)
		sclk = get_pll_clk(MPLL);
	else if (sel == 0x7)
		sclk = get_pll_clk(EPLL);
	else if (sel == 0x8)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	/*
	 * CLK_DIV_PERIC0
	 * UART0_RATIO [3:0]
	 * UART1_RATIO [7:4]
	 * UART2_RATIO [8:11]
	 * UART3_RATIO [12:15]
	 * UART4_RATIO [16:19]
	 * UART5_RATIO [23:20]
	 */
	ratio = readl(&clk->div_peric0);
	ratio = (ratio >> (dev_index << 2)) & 0xf;

	uclk = sclk / (ratio + 1);

	return uclk;
}

/* exynos5: set the mmc clock */
void set_mmc_clk(int dev_index, unsigned int div)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned int addr;
	unsigned int val;

	/*
	 * CLK_DIV_FSYS1
	 * MMC0_PRE_RATIO [15:8], MMC1_PRE_RATIO [31:24]
	 * CLK_DIV_FSYS2
	 * MMC2_PRE_RATIO [15:8], MMC3_PRE_RATIO [31:24]
	 */
	if (dev_index < 2) {
		addr = (unsigned int)&clk->div_fsys1;
	} else {
		addr = (unsigned int)&clk->div_fsys2;
		dev_index -= 2;
	}

	val = readl(addr);
	val &= ~(0xff << ((dev_index << 4) + 8));
	val |= (div & 0xff) << ((dev_index << 4) + 8);
	writel(val, addr);
}

int get_mshci_clk_div(enum periph_id peripheral)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	u32 *addr;
	unsigned int div_mmc, div_mmc_pre;
	unsigned int mpll_clock, sclk_mmc;

	mpll_clock = get_pll_clk(MPLL);

	/*
	 * CLK_DIV_FSYS1
	 * MMC0_PRE_RATIO [15:8]
	 * MMC0_RATIO [3:0]
	 * CLK_DIV_FSYS2
	 * MMC2_PRE_RATIO [15:8]
	 * MMC2_RATIO [3:0]
	 */
	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
		addr = &clk->div_fsys1;
		break;
	case PERIPH_ID_SDMMC2:
		addr = &clk->div_fsys2;
		break;
	default:
		debug("invalid peripheral\n");
		return -1;
	}

	div_mmc = (readl(addr) & 0xf) + 1;
	div_mmc_pre = ((readl(addr) & 0xff00) >> 8) + 1;

	sclk_mmc = (mpll_clock / div_mmc) / div_mmc_pre;

	return sclk_mmc;
}

/* exynos5: obtaining the I2C clock */
unsigned long get_i2c_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long aclk_66, aclk_66_pre, sclk;
	unsigned int ratio;

	sclk = get_pll_clk(MPLL);

	ratio = ((readl(&clk->div_top1)) >> 24);
	ratio &= (0x7);
	aclk_66_pre = sclk/(ratio+1);
	ratio = readl(&clk->div_top0);
	ratio &= (0x7);
	aclk_66 = aclk_66_pre/(ratio+1);
	return aclk_66;
}

void clock_ll_set_pre_ratio(enum periph_id periph_id, unsigned divisor)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned shift;
	unsigned mask = 0xff;
	u32 *reg;

	/*
	 * For now we only handle a very small subset of peipherals here.
	 * Others will need to (and do) mangle the clock registers
	 * themselves, At some point it is hoped that this function can work
	 * from a table or calculated register offset / mask. For now this
	 * is at least better than spreading clock control code around
	 * U-Boot.
	 */
	switch (periph_id) {
	case PERIPH_ID_SPI0:
		reg = &clk->div_peric1;
		shift = 8;
		break;
	case PERIPH_ID_SPI1:
		reg = &clk->div_peric1;
		shift = 24;
		break;
	case PERIPH_ID_SPI2:
		reg = &clk->div_peric2;
		shift = 8;
		break;
	case PERIPH_ID_SPI3:
		reg = &clk->sclk_div_isp;
		shift = 4;
		break;
	case PERIPH_ID_SPI4:
		reg = &clk->sclk_div_isp;
		shift = 16;
		break;
	default:
		debug("%s: Unsupported peripheral ID %d\n", __func__,
		      periph_id);
		return;
	}
	clrsetbits_le32(reg, mask << shift, (divisor & mask) << shift);
}

void clock_ll_set_ratio(enum periph_id periph_id, unsigned divisor)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned shift;
	unsigned mask = 0xff;
	u32 *reg;

	switch (periph_id) {
	case PERIPH_ID_SPI0:
		reg = &clk->div_peric1;
		shift = 0;
		break;
	case PERIPH_ID_SPI1:
		reg = &clk->div_peric1;
		shift = 16;
		break;
	case PERIPH_ID_SPI2:
		reg = &clk->div_peric2;
		shift = 0;
		break;
	case PERIPH_ID_SPI3:
		reg = &clk->sclk_div_isp;
		shift = 0;
		break;
	case PERIPH_ID_SPI4:
		reg = &clk->sclk_div_isp;
		shift = 12;
		break;
	default:
		debug("%s: Unsupported peripheral ID %d\n", __func__,
		      periph_id);
		return;
	}
	clrsetbits_le32(reg, mask << shift, (divisor & mask) << shift);
}

/**
 * Linearly searches for the most accurate main and fine stage clock scalars
 * (divisors) for a specified target frequency and scalar bit sizes by checking
 * all multiples of main_scalar_bits values. Will always return scalars up to or
 * slower than target.
 *
 * @param main_scalar_bits	Number of main scalar bits, must be > 0 and < 32
 * @param fine_scalar_bits	Number of fine scalar bits, must be > 0 and < 32
 * @param input_freq		Clock frequency to be scaled in Hz
 * @param target_freq		Desired clock frequency in Hz
 * @param best_fine_scalar	Pointer to store the fine stage divisor
 *
 * @return best_main_scalar	Main scalar for desired frequency or -1 if none
 * found
 */
static int clock_calc_best_scalar(unsigned int main_scaler_bits,
	unsigned int fine_scalar_bits, unsigned int input_rate,
	unsigned int target_rate, unsigned int *best_fine_scalar)
{
	int i;
	int best_main_scalar = -1;
	unsigned int best_error = target_rate;
	const unsigned int cap = (1 << fine_scalar_bits) - 1;
	const unsigned int loops = 1 << main_scaler_bits;

	debug("Input Rate is %u, Target is %u, Cap is %u\n", input_rate,
			target_rate, cap);

	assert(best_fine_scalar != NULL);
	assert(main_scaler_bits <= fine_scalar_bits);

	*best_fine_scalar = 1;

	if (input_rate == 0 || target_rate == 0)
		return -1;

	if (target_rate >= input_rate)
		return 1;

	for (i = 1; i <= loops; i++) {
		const unsigned int effective_div = max(min(input_rate / i /
							target_rate, cap), 1);
		const unsigned int effective_rate = input_rate / i /
							effective_div;
		const int error = target_rate - effective_rate;

		debug("%d|effdiv:%u, effrate:%u, error:%d\n", i, effective_div,
				effective_rate, error);

		if (error >= 0 && error <= best_error) {
			best_error = error;
			best_main_scalar = i;
			*best_fine_scalar = effective_div;
		}
	}

	return best_main_scalar;
}

int clock_set_rate(enum periph_id periph_id, unsigned int rate)
{
	int main;
	unsigned int fine;

	switch (periph_id) {
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
	case PERIPH_ID_SPI3:
	case PERIPH_ID_SPI4:
		main = clock_calc_best_scalar(4, 8, 400000000, rate, &fine);
		if (main < 0) {
			debug("%s: Cannot set clock rate for periph %d",
					__func__, periph_id);
			return -1;
		}
		clock_ll_set_ratio(periph_id, main - 1);
		clock_ll_set_pre_ratio(periph_id, fine - 1);
		break;
	default:
		debug("%s: Unsupported peripheral ID %d\n", __func__,
		      periph_id);
		return -1;
	}

	return 0;
}

int clock_set_mshci(enum periph_id peripheral)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	u32 *addr;
	unsigned int clock;
	unsigned int tmp;
	unsigned int i;

	/* get mpll clock */
	clock = get_pll_clk(MPLL) / 1000000;

	/*
	 * CLK_DIV_FSYS1
	 * MMC0_PRE_RATIO [15:8], MMC0_RATIO [3:0]
	 * CLK_DIV_FSYS2
	 * MMC2_PRE_RATIO [15:8], MMC2_RATIO [3:0]
	 */
	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
		addr = &clk->div_fsys1;
		break;
	case PERIPH_ID_SDMMC2:
		addr = &clk->div_fsys2;
		break;
	default:
		debug("invalid peripheral\n");
		return -1;
	}
	tmp = readl(addr) & ~0xff0f;
	for (i = 0; i <= 0xf; i++) {
		if ((clock / (i + 1)) <= 400) {
			writel(tmp | i << 0, addr);
			break;
		}
	}
	return 0;
}

#ifdef CONFIG_OF_CONTROL
int clock_decode_periph_id(const void *blob, int node)
{
	enum periph_id id;

	/*
	 * For now the peripheral ID is directly encoded. Once we have clock
	 * support in the fdt and properly in exynos U-Boot we may have
	 * another way of changing the clock.
	 */
	id = fdtdec_get_int(blob, node, "samsung,periph-id", -1);
	assert(id != PERIPH_ID_NONE);
	assert(id >= 0 && id < PERIPH_ID_COUNT);

	return id;
}
#endif
