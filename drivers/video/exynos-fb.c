/*
 * LCD driver for Exynos
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/clk.h>
#include <asm/arch/dsim.h>
#include <asm/arch/fimd.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/pwm.h>
#include <asm/arch/sysreg.h>
#include <lcd.h>
#include <pwm.h>
#include <asm/unaligned.h>

/* MIPI DSI Processor-to-Peripheral transaction types */
enum {
	/* Add other types here as and when required */
	MIPI_DSI_GENERIC_LONG_WRITE	= 0x29,
};

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;		/* Start of framebuffer memory */
void *lcd_console_address;	/* Start of console buffer */

short console_col;
short console_row;

vidinfo_t panel_info = {
	.vl_col		= LCD_XRES,
	.vl_row		= LCD_YRES,
	.vl_bpix	= LCD_COLOR16,
};

void lcd_show_board_info()
{
	return;
}

void lcd_enable()
{
	return;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}

/* Bypass FIMD of DISP1_BLK */
static void fimd_bypass(void)
{
	struct exynos5_sysreg *sysreg =
		(struct exynos5_sysreg *)samsung_get_base_sysreg();

	setbits_le32(&sysreg->disp1blk_cfg, FIMDBYPASS_DISP1);
}

/* Calculate the size of Framebuffer from the resolution */
ulong calc_fbsize(void)
{
	return ALIGN((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix) / 8), PAGE_SIZE);
}

/*
 * Enables the PLL of the MIPI-DSIM block.
 *
 * Depending upon the flag "enable" we will either enable the PLL or disable it
 *
 * @param dsim		pointer to the MIPI-DSIM register base address
 * @param enable	Flag which states whether or not to enable the PLL
 */
static void mipi_dsi_enable_pll(struct exynos5_dsim *dsim,
					unsigned int enable)
{
	clrbits_le32(&dsim->pllctrl, DSIM_PLL_EN_SHIFT);

	if (enable)
		setbits_le32(&dsim->pllctrl, DSIM_PLL_EN_SHIFT);
}

/*
 * Clear the MIPI-DSIM interrupt bit.
 *
 * The functions clear the interrupt bit of the DSIM interrupt status register
 *
 * @param dsim		pointer to the MIPI-DSIM register base address
 * @param int_src	Interrupt bit to clear.
 */
static void mipi_dsi_clear_interrupt(struct exynos5_dsim *dsim,
						unsigned int int_src)
{
	writel(int_src, &dsim->intsrc);
}

/*
 * Check whether D-phy generates stable byteclk.
 *
 * @param dsim	pointer to the MIPI-DSIM register base address
 * @return	non-zero when byteclk is stable; 0 otherwise
 */
static unsigned int mipi_dsi_is_pll_stable(struct exynos5_dsim *dsim)
{
	unsigned int reg;

	reg = readl(&dsim->status);

	return reg & PLL_STABLE;
}

/*
 * Write DSIM packet header.
 *
 * @param dsim		pointer to the MIPI-DSIM register base address
 * @param di		indicates the packet type
 * @param size		size of payload data in bytes
 */
static void mipi_dsi_wr_tx_header(struct exynos5_dsim *dsim,
		unsigned int di, unsigned int size)
{
	unsigned int reg = (size  << 8) | ((di & 0x3f) << 0);

	writel(reg, &dsim->pkthdr);
}

/*
 * Write whole words of DSIM packet to the payload register.
 * The caller does not need to make sure that data is aligned,
 * also the caller must ensure that reading a whole number of words
 * does not create any side-effects. This means that data always contain
 * multiple of 4 bytes, even if size is not a multiple of 4.
 *
 * @param dsim          pointer to the MIPI-DSIM register base address
 * @param data          pointer to payload data
 * @param size          size of payload data in bytes
 */
static void mipi_dsi_long_data_wr(struct exynos5_dsim *dsim,
		unsigned char *data, unsigned int size)
{
	unsigned int data_cnt, payload;

	for (data_cnt = 0; data_cnt < size; data_cnt += 4, data += 4) {
		payload = get_unaligned((u32 *)data);
		writel(payload, &dsim->payload);
	}
}

/*
 * Write MIPI-DSI payload data and packet header. Then wait till that data goes
 * out and the FIFO_EMPTY interrupt comes.
 *
 * @param dsim		pointer to the MIPI-DSIM register base address
 * @param data_id	indicates the packet type
 * @param data		pointer to payload data
 * @param size		size of payload data in bytes
 */
static void mipi_dsi_wr_data(struct exynos5_dsim *dsim,
	unsigned int data_id, unsigned char *data, unsigned int size)
{
	mipi_dsi_long_data_wr(dsim, data, size);

	/* put data into header fifo */
	mipi_dsi_wr_tx_header(dsim, data_id, size);

	/*
	 * TODO: Find out a better solution to eliminate this delay.
	 *
	 * Currently the delay is added to provide some time for the
	 * SFR payload FIFO to get empty.
	 */
	mdelay(2);
}

/*
 * Wait till all the lanes are in STOP state or we are ready to transmit HS
 * data at clock lane
 *
 * @param dsim	pointer to the MIPI-DSIM register base address
 * @return	1 if the above stated condition is true; 0 otherwise
 */
static int mipi_dsi_is_lane_state(struct exynos5_dsim *dsim)
{
	unsigned int reg = readl(&dsim->status);

	if ((reg & DSIM_STOP_STATE_DAT(0xf)) &&
		((reg & DSIM_STOP_STATE_CLK) ||
		(reg & DSIM_TX_READY_HS_CLK)))
		return 1;
	else
		return 0;
}

/*
 * Initialize the LCD panel.
 *
 * @param dsim	pointer to the MIPI-DSIM register base address
 */
static void init_lcd(struct exynos5_dsim *dsim)
{
	int i;
	unsigned char initcode[][6] = {
		{0x3c, 0x01, 0x03, 0x00, 0x02, 0x00},
		{0x14, 0x01, 0x02, 0x00, 0x00, 0x00},
		{0x64, 0x01, 0x05, 0x00, 0x00, 0x00},
		{0x68, 0x01, 0x05, 0x00, 0x00, 0x00},
		{0x6c, 0x01, 0x05, 0x00, 0x00, 0x00},
		{0x70, 0x01, 0x05, 0x00, 0x00, 0x00},
		{0x34, 0x01, 0x1f, 0x00, 0x00, 0x00},
		{0x10, 0x02, 0x1f, 0x00, 0x00, 0x00},
		{0x04, 0x01, 0x01, 0x00, 0x00, 0x00},
		{0x04, 0x02, 0x01, 0x00, 0x00, 0x00},
		{0x50, 0x04, 0x20, 0x01, 0xfa, 0x00},
		{0x54, 0x04, 0x20, 0x00, 0x50, 0x00},
		{0x58, 0x04, 0x00, 0x05, 0x30, 0x00},
		{0x5c, 0x04, 0x05, 0x00, 0x0a, 0x00},
		{0x60, 0x04, 0x20, 0x03, 0x0a, 0x00},
		{0x64, 0x04, 0x01, 0x00, 0x00, 0x00},
		{0xa0, 0x04, 0x06, 0x80, 0x44, 0x00},
		{0xa0, 0x04, 0x06, 0x80, 0x04, 0x00},
		{0x04, 0x05, 0x04, 0x00, 0x00, 0x00},
		{0x9c, 0x04, 0x0d, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	};

	for (i = 0; i < (ARRAY_SIZE(initcode)-1); i++)
		mipi_dsi_wr_data(dsim, MIPI_DSI_GENERIC_LONG_WRITE,
			initcode[i], sizeof(initcode[i]));
}

/*
 * Initialize MIPI DSI
 */
static void mipi_init(void)
{
	int sw_timeout, timeout;
	unsigned int val, pms;
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();
	struct exynos5_dsim *dsim =
		(struct exynos5_dsim *)samsung_get_base_dsim();

	/* Reset DSIM and enable MIPI_PHY1 */
	val = MIPI_PHY1_CONTROL_ENABLE | MIPI_PHY1_CONTROL_M_RESETN;
	writel(val, &power->mipi_phy1_control);

	writel(DSIM_SWRST, &dsim->swrst);

	/* Enabling data lane 0-3 */
	val = ENABLE_ALL_DATA_LANE | NUM_OF_DAT_LANE_IS_FOUR;
	writel(val, &dsim->config);

	/* Enable AFC with value 0x3 for MIPI DPHY */
	val = DSIM_PHYACCHR_AFC_CTL_VAL << DSIM_PHYACCHR_AFC_CTL_OFFSET;
	val |= DSIM_PHYACCHR_AFC_EN;
	writel(val, &dsim->phyacchr);

	pms = DSIM_PLLCTRL_PMS_VAL << DSIM_PLLCTRL_PMS_OFFSET;
	val = DSIM_FREQ_BAND << DSIM_FREQ_BAND_OFFSET;
	writel((val | pms), &dsim->pllctrl);

	writel(DSIM_PLLTMR_VAL, &dsim->plltmr);

	sw_timeout = 1000;
	mipi_dsi_clear_interrupt(dsim, PLL_STABLE);
	mipi_dsi_enable_pll(dsim, ENABLE);

	while (sw_timeout) {
		sw_timeout--;
		if (mipi_dsi_is_pll_stable(dsim))
			break;
	}

	/* Enable escape clk
	 * enable HS clk
	 * Enable Byte clk
	 * Set escape clk prescalar value to 0x90
	 */
	val = DSIM_ESC_PRESCALAR_VAL | LANE_ESC_CLK_EN_ALL |
		BYTE_CLK_EN;/* | DSIM_ESC_CLK_EN;*/
	writel(val, &dsim->clkctrl);

	timeout =  100;
	/* Wait for the Data & clock lane to go in Stop state */
	while (!(mipi_dsi_is_lane_state(dsim)))
		timeout--;

	/* set_stop_state_counter */
	val = STOP_STATE_CNT_VAL << STOP_STATE_CNT_OFFSET;
	writel(val, &dsim->escmode);

	setbits_le32(&dsim->clkctrl, TXREQUEST_HS_CLK_ON);

	setbits_le32(&dsim->escmode, LP_MODE_ENABLE);

	val = (MAIN_VBP_VAL << MAIN_VBP_OFFSET) |
		(STABLE_VFP_VAL << STABLE_VFP_OFFSET) |
		(CMD_ALLOW_VAL << CMD_ALLOW_OFFSET);
	writel(val, &dsim->mvporch);

	val = (MAIN_HFP_VAL << MAIN_HFP_OFFSET) |
		(MAIN_HBP_VAL << MAIN_HBP_OFFSET);
	writel(val, &dsim->mhporch);

	val = (MAIN_HSA_VAL << MAIN_HSA_OFFSET) |
		(MAIN_VSA_VAL << MAIN_VSA_OFFSET);
	writel(val, &dsim->msync);

	val = (MAIN_VRESOL_VAL << MAIN_VRESOL_OFFSET) |
		(MAIN_HRESOL_VAL << MAIN_HRESOL_OFFSET);
	val |= MAIN_STANDBY;
	writel(val, &dsim->mdresol);

	val = readl(&dsim->config);
	val = ENABLE_ALL_DATA_LANE | NUM_OF_DAT_LANE_IS_FOUR | CLK_LANE_EN;
	val |= (RGB_565_16_BIT << MAIN_PIX_FORMAT_OFFSET);
	val |= BURST_MODE | VIDEO_MODE;
	writel(val, &dsim->config);

	writel(SFR_FIFO_EMPTY, &dsim->intsrc);
	init_lcd(dsim);
	clrbits_le32(&dsim->escmode, LP_MODE_ENABLE);
}

/*
 * Initialize display controller.
 *
 * @param lcdbase	pointer to the base address of framebuffer.
 */
static void fb_init(void *lcdbase)
{
	unsigned int val;
	ulong fbsize;
	struct exynos5_fimd *fimd =
		(struct exynos5_fimd *)samsung_get_base_fimd();
	struct exynos5_disp_ctrl *disp_ctrl =
		(struct exynos5_disp_ctrl *)samsung_get_base_disp_ctrl();

	writel(VCLK_RISING_EDGE | VCLK_RUNNING, &disp_ctrl->vidcon1);

	val = ENVID_ON | ENVID_F_ON | (CLKVAL_F << CLKVAL_F_OFFSET);
	writel(val, &fimd->vidcon0);

	val = (VSYNC_PULSE_WIDTH_VAL << VSYNC_PULSE_WIDTH_OFFSET) |
		(V_FRONT_PORCH_VAL << V_FRONT_PORCH_OFFSET) |
		(V_BACK_PORCH_VAL << V_BACK_PORCH_OFFSET);
	writel(val, &disp_ctrl->vidtcon0);

	val = (HSYNC_PULSE_WIDTH_VAL << HSYNC_PULSE_WIDTH_OFFSET) |
		(H_FRONT_PORCH_VAL << H_FRONT_PORCH_OFFSET) |
		(H_BACK_PORCH_VAL << H_BACK_PORCH_OFFSET);
	writel(val, &disp_ctrl->vidtcon1);

	val = ((LCD_XRES - 1) << HOZVAL_OFFSET) |
		((LCD_YRES - 1) << LINEVAL_OFFSET);
	writel(val, &disp_ctrl->vidtcon2);

	writel((unsigned int)lcd_base, &fimd->vidw00add0b0);

	fbsize = calc_fbsize();
	writel((unsigned int)lcd_base + fbsize, &fimd->vidw00add1b0);

	writel(LCD_XRES * 2, &fimd->vidw00add2);

	val = ((LCD_XRES - 1) << OSD_RIGHTBOTX_F_OFFSET);
	val |= ((LCD_YRES - 1) << OSD_RIGHTBOTY_F_OFFSET);
	writel(val, &fimd->vidosd0b);
	writel(LCD_XRES * LCD_YRES, &fimd->vidosd0c);

	setbits_le32(&fimd->shadowcon, CHANNEL0_EN);

	val = BPPMODE_F_RGB_16BIT_565 << BPPMODE_F_OFFSET;
	val |= ENWIN_F_ENABLE | HALF_WORD_SWAP_EN;
	writel(val, &fimd->wincon0);
}

void exynos_fimd_disable(void)
{
	struct exynos5_fimd *fimd =
		(struct exynos5_fimd *)samsung_get_base_fimd();

	writel(0, &fimd->wincon0);
	clrbits_le32(&fimd->shadowcon, CHANNEL0_EN);
}

void lcd_ctrl_init(void *lcdbase)
{
	exynos_pinmux_config(PERIPH_ID_BACKLIGHT, 0);
	exynos_pinmux_config(PERIPH_ID_LCD, 0);
	pwm_init(0, MUX_DIV_2, 0);
	mipi_init();
	fimd_bypass();
	fb_init(lcdbase);
}
