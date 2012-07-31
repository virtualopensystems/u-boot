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
#include <fdtdec.h>
#include <lcd.h>
#include <pwm.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/clk.h>
#include <asm/arch/dsim.h>
#include <asm/arch/fimd.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/pwm.h>
#include <asm/arch/s5p-dp.h>
#include <asm/arch/sysreg.h>
#include <asm/arch-exynos/spl.h>

#include "s5p-dp-core.h"

DECLARE_GLOBAL_DATA_PTR;

/* To help debug any init errors here, define a list of possible errors */
enum {
	ERR_PLL_NOT_UNLOCKED = 2,
	ERR_VIDEO_CLOCK_BAD,
	ERR_VIDEO_STREAM_BAD,
	ERR_DPCD_READ_ERROR1,		/* 5 */

	ERR_DPCD_WRITE_ERROR1,
	ERR_DPCD_READ_ERROR2,
	ERR_DPCD_WRITE_ERROR2,
	ERR_INVALID_LANE,
	ERR_PLL_NOT_LOCKED,		/* 10 */

	ERR_PRE_EMPHASIS_LEVELS,
	ERR_LINK_RATE_ABNORMAL,
	ERR_MAX_LANE_COUNT_ABNORMAL,
	ERR_LINK_TRAINING_FAILURE,
	ERR_MISSING_DP_BASE,		/* 15 */

	ERR_NO_FDT_NODE,
};

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

static ushort cmap_buffer[1 << 16];

vidinfo_t panel_info = {
	.vl_col		= LCD_XRES,
	.vl_row		= LCD_YRES,
	.vl_bpix	= LCD_COLOR16,
	.cmap		= cmap_buffer,
};

static struct exynos5_fimd_panel global_panel_data[2] = {
	{
		/* Display I/F is eDP */
		.is_dp = 1,
		.is_mipi = 0,
		.fixvclk = 0,
		.ivclk = 0,
		.clkval_f = 2,
		.upper_margin = 14,
		.lower_margin = 3,
		.vsync = 5,
		.left_margin = 80,
		.right_margin = 48,
		.hsync = 32,
	}, {
		/* Display I/F is MIPI */
		.is_dp = 0,
		.is_mipi = 1,
		.fixvclk = 1,
		.ivclk = 1,
		.clkval_f = 0xb,
		.upper_margin = 3,
		.lower_margin = 3,
		.vsync = 3,
		.left_margin = 3,
		.right_margin = 3,
		.hsync = 3,
	}
};

static struct s5p_dp_device dp_device;

static struct video_info smdk5250_dp_config = {
	.name			= "eDP-LVDS NXP PTN3460",

	.h_sync_polarity	= 0,
	.v_sync_polarity	= 0,
	.interlaced		= 0,

	.color_space		= COLOR_RGB,
	.dynamic_range		= VESA,
	.ycbcr_coeff		= COLOR_YCBCR601,
	.color_depth		= COLOR_8,

	.link_rate		= LINK_RATE_2_70GBPS,
	.lane_count		= LANE_COUNT2,
};

void lcd_show_board_info()
{
}

void lcd_enable()
{
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
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

	/*
	 * De-assert LVDS_RESET_L
	 *
	 * TODO(dianders): This need to move to the device tree.
	 * TODO(dianders): If we get MIPI working in U-Boot again, need to
	 * test that this actually works properly.  Kernel code indicates
	 * that maybe we should be pulsing this line rather than driving it?
	 * ...and maybe we need a delay after doing this?
	 */
	gpio_set_value(GPIO_X15, 1);
	gpio_cfg_pin(GPIO_X15, EXYNOS_GPIO_OUTPUT);
	gpio_set_pull(GPIO_X15, EXYNOS_GPIO_PULL_NONE);

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
 * @pd			pointer to the main panel_data structure
 */
static void fb_init(void *lcdbase, struct exynos5_fimd_panel *pd)
{
	unsigned int val;
	ulong fbsize;
	struct exynos5_fimd *fimd =
		(struct exynos5_fimd *)samsung_get_base_fimd();
	struct exynos5_disp_ctrl *disp_ctrl =
		(struct exynos5_disp_ctrl *)samsung_get_base_disp_ctrl();

	writel(pd->ivclk | pd->fixvclk, &disp_ctrl->vidcon1);
	val = ENVID_ON | ENVID_F_ON | (pd->clkval_f << CLKVAL_F_OFFSET);
	writel(val, &fimd->vidcon0);

	val = (pd->vsync << VSYNC_PULSE_WIDTH_OFFSET) |
		(pd->lower_margin << V_FRONT_PORCH_OFFSET) |
		(pd->upper_margin << V_BACK_PORCH_OFFSET);
	writel(val, &disp_ctrl->vidtcon0);

	val = (pd->hsync << HSYNC_PULSE_WIDTH_OFFSET) |
		(pd->right_margin << H_FRONT_PORCH_OFFSET) |
		(pd->left_margin << H_BACK_PORCH_OFFSET);
	writel(val, &disp_ctrl->vidtcon1);

	val = ((pd->xres - 1) << HOZVAL_OFFSET) |
		((pd->yres - 1) << LINEVAL_OFFSET);
	writel(val, &disp_ctrl->vidtcon2);

	writel((unsigned int)lcd_base, &fimd->vidw00add0b0);

	fbsize = calc_fbsize();
	writel((unsigned int)lcd_base + fbsize, &fimd->vidw00add1b0);

	writel(pd->xres * 2, &fimd->vidw00add2);

	val = ((pd->xres - 1) << OSD_RIGHTBOTX_F_OFFSET);
	val |= ((pd->yres - 1) << OSD_RIGHTBOTY_F_OFFSET);
	writel(val, &fimd->vidosd0b);
	writel(pd->xres * pd->yres, &fimd->vidosd0c);

	setbits_le32(&fimd->shadowcon, CHANNEL0_EN);

	val = BPPMODE_F_RGB_16BIT_565 << BPPMODE_F_OFFSET;
	val |= ENWIN_F_ENABLE | HALF_WORD_SWAP_EN;
	writel(val, &fimd->wincon0);

	/* DPCLKCON_ENABLE */
	writel(1 << 1, &fimd->dpclkcon);
}

void exynos_fimd_disable(void)
{
	struct exynos5_fimd *fimd =
		(struct exynos5_fimd *)samsung_get_base_fimd();

	writel(0, &fimd->wincon0);
	clrbits_le32(&fimd->shadowcon, CHANNEL0_EN);
}

/*
 * Configure DP in slave mode and wait for video stream.
 *
 * param dp		pointer to main s5p-dp structure
 * param video_info	pointer to main video_info structure.
 * return		status
 */
static int s5p_dp_config_video(struct s5p_dp_device *dp,
			struct video_info *video_info)
{
	int timeout = 0;
	ulong start;
	struct exynos5_dp *base = dp->base;

	s5p_dp_config_video_slave_mode(dp, video_info);

	s5p_dp_set_video_color_format(dp, video_info->color_depth,
			video_info->color_space,
			video_info->dynamic_range,
			video_info->ycbcr_coeff);

	if (s5p_dp_get_pll_lock_status(dp) == PLL_UNLOCKED) {
		debug("PLL is not locked yet.\n");
		return -ERR_PLL_NOT_UNLOCKED;
	}

	start = get_timer(0);
	do {
		if (s5p_dp_is_slave_video_stream_clock_on(dp) == 0) {
			timeout++;
			break;
		}
	} while (get_timer(start) <= STREAM_ON_TIMEOUT);

	if (!timeout) {
		debug("Video Clock Not ok\n");
		return -ERR_VIDEO_CLOCK_BAD;
	}

	/* Set to use the register calculated M/N video */
	s5p_dp_set_video_cr_mn(dp, CALCULATED_M, 0, 0);

	clrbits_le32(&base->video_ctl_10, FORMAT_SEL);

	/* Disable video mute */
	clrbits_le32(&base->video_ctl_1, HDCP_VIDEO_MUTE);

	/* Configure video slave mode */
	s5p_dp_enable_video_master(dp);

	/* Enable video */
	setbits_le32(&base->video_ctl_1, VIDEO_EN);
	timeout = s5p_dp_is_video_stream_on(dp);

	if (timeout) {
		debug("Video Stream Not on\n");
		return -ERR_VIDEO_STREAM_BAD;
	}

	return 0;
}

/*
 * Set DP to enhanced mode. We use this for EVT1
 * param dp	pointer to main s5p-dp structure
 * return	status
 */
static int s5p_dp_enable_rx_to_enhanced_mode(struct s5p_dp_device *dp)
{
	u8 data;

	if (s5p_dp_read_byte_from_dpcd(dp, DPCD_ADDR_LANE_COUNT_SET, &data)) {
		debug("DPCD read error\n");
		return -ERR_DPCD_READ_ERROR1;
	}

	if (s5p_dp_write_byte_to_dpcd(dp, DPCD_ADDR_LANE_COUNT_SET,
					DPCD_ENHANCED_FRAME_EN |
					(data & DPCD_LANE_COUNT_SET_MASK))) {
		debug("DPCD write error\n");
		return -ERR_DPCD_WRITE_ERROR1;
	}

	return 0;
}

/*
 * Enable scrambles mode. We use this for EVT1
 * param dp	pointer to main s5p-dp structure
 * return	status
 */
static int s5p_dp_enable_scramble(struct s5p_dp_device *dp)
{
	u8 data;
	struct exynos5_dp *base = dp->base;

	clrbits_le32(&base->dp_training_ptn_set, SCRAMBLING_DISABLE);

	if (s5p_dp_read_byte_from_dpcd(dp, DPCD_ADDR_TRAINING_PATTERN_SET,
								&data)) {
		debug("DPCD read error\n");
		return -ERR_DPCD_READ_ERROR2;
	}

	if (s5p_dp_write_byte_to_dpcd(dp, DPCD_ADDR_TRAINING_PATTERN_SET,
				(u8)(data & ~DPCD_SCRAMBLING_DISABLED))) {
		debug("DPCD write error\n");
		return -ERR_DPCD_WRITE_ERROR2;
	}

	return 0;
}

/*
 * Reset DP and prepare DP for init training
 * param dp	pointer to main s5p-dp structure
 */
static int s5p_dp_init_dp(struct s5p_dp_device *dp)
{
	int ret;
	struct exynos5_dp *base = dp->base;

	s5p_dp_reset(dp);

	/* SW defined function Normal operation */
	clrbits_le32(&base->func_en_1, SW_FUNC_EN_N);

	ret = s5p_dp_init_analog_func(dp);
	if (ret)
		return ret;

	s5p_dp_init_aux(dp);

	return ret;
}

/*
 * Set pre-emphasis level
 * param dp		pointer to main s5p-dp structure
 * param pre_emphasis	pre-emphasis level
 * param lane		lane number(0 - 3)
 * return		status
 */
static int s5p_dp_set_lane_lane_pre_emphasis(struct s5p_dp_device *dp,
					int pre_emphasis, int lane)
{
	u32 reg;
	struct exynos5_dp *base = dp->base;

	reg = pre_emphasis << PRE_EMPHASIS_SET_SHIFT;
	switch (lane) {
	case 0:
		writel(reg, &base->ln0_link_trn_ctl);
		break;
	case 1:
		writel(reg, &base->ln1_link_trn_ctl);
		break;

	case 2:
		writel(reg, &base->ln2_link_trn_ctl);
		break;

	case 3:
		writel(reg, &base->ln3_link_trn_ctl);
		break;
	default:
		debug("%s: Invalid lane %d\n", __func__, lane);
		return -ERR_INVALID_LANE;
	}
	return 0;
}

/*
 * Read supported bandwidth type
 * param dp		pointer to main s5p-dp structure
 * param bandwidth	pointer to variable holding bandwidth type
 */
static void s5p_dp_get_max_rx_bandwidth(struct s5p_dp_device *dp,
			u8 *bandwidth)
{
	u8 data;

	/*
	 * For DP rev.1.1, Maximum link rate of Main Link lanes
	 * 0x06 = 1.62 Gbps, 0x0a = 2.7 Gbps
	 */
	s5p_dp_read_byte_from_dpcd(dp, DPCD_ADDR_MAX_LINK_RATE, &data);
	*bandwidth = data;
}

/*
 * Reset DP and prepare DP for init training
 * param dp		pointer to main s5p-dp structure
 * param lane_count	pointer to variable holding no of lanes
 */
static void s5p_dp_get_max_rx_lane_count(struct s5p_dp_device *dp,
			u8 *lane_count)
{
	u8 data;

	/*
	 * For DP rev.1.1, Maximum number of Main Link lanes
	 * 0x01 = 1 lane, 0x02 = 2 lanes, 0x04 = 4 lanes
	 */
	s5p_dp_read_byte_from_dpcd(dp, DPCD_ADDR_MAX_LANE_COUNT, &data);
	*lane_count = data & DPCD_MAX_LANE_COUNT_MASK;
}

/*
 * DP H/w Link Training. Set DPCD link rate and bandwidth.
 * param dp		pointer to main s5p-dp structure
 * param max_lane	No of lanes
 * param max_rate	bandwidth
 * return status
 */
static int s5p_dp_hw_link_training(struct s5p_dp_device *dp,
				unsigned int max_lane,
				unsigned int max_rate)
{
	u32 data;
	int lane;
	struct exynos5_dp *base = dp->base;

	/* Stop Video */
	clrbits_le32(&base->video_ctl_1, VIDEO_EN);

	if (s5p_dp_get_pll_lock_status(dp) == PLL_UNLOCKED) {
		debug("PLL is not locked yet.\n");
		return -ERR_PLL_NOT_LOCKED;
	}

	/* Reset Macro */
	setbits_le32(&base->dp_phy_test, MACRO_RST);

	/* 10 us is the minimum reset time. */
	udelay(10);

	clrbits_le32(&base->dp_phy_test, MACRO_RST);

	/* Set TX pre-emphasis to minimum */
	for (lane = 0; lane < max_lane; lane++)
		if (s5p_dp_set_lane_lane_pre_emphasis(dp,
			PRE_EMPHASIS_LEVEL_0, lane)) {
			debug("Unable to set pre emphasis level\n");
			return -ERR_PRE_EMPHASIS_LEVELS;
		}

	/* All DP analog module power up */
	writel(0x00, &base->dp_phy_pd);

	/* Initialize by reading RX's DPCD */
	s5p_dp_get_max_rx_bandwidth(dp, &dp->link_train.link_rate);
	s5p_dp_get_max_rx_lane_count(dp, &dp->link_train.lane_count);

	if ((dp->link_train.link_rate != LINK_RATE_1_62GBPS) &&
		(dp->link_train.link_rate != LINK_RATE_2_70GBPS)) {
		debug("Rx Max Link Rate is abnormal :%x !\n",
			dp->link_train.link_rate);
		/* Not Retrying */
		return -ERR_LINK_RATE_ABNORMAL;
	}

	if (dp->link_train.lane_count == 0) {
		debug("Rx Max Lane count is abnormal :%x !\n",
			dp->link_train.lane_count);
		/* Not retrying */
		return -ERR_MAX_LANE_COUNT_ABNORMAL;
	}

	/* Setup TX lane count & rate */
	if (dp->link_train.lane_count > max_lane)
		dp->link_train.lane_count = max_lane;
	if (dp->link_train.link_rate > max_rate)
		dp->link_train.link_rate = max_rate;

	/* Set link rate and count as you want to establish*/
	writel(dp->video_info->lane_count, &base->lane_count_set);
	writel(dp->video_info->link_rate, &base->link_bw_set);

	/* Set sink to D0 (Sink Not Ready) mode. */
	s5p_dp_write_byte_to_dpcd(dp, DPCD_ADDR_SINK_POWER_STATE,
				DPCD_SET_POWER_STATE_D0);

	/* Start HW link training */
	writel(HW_TRAINING_EN, &base->dp_hw_link_training);

	/* Wait unitl HW link training done */
	s5p_dp_wait_hw_link_training_done(dp);

	/* Get hardware link training status */
	data = readl(&base->dp_hw_link_training);
	if (data != 0) {
		debug(" H/W link training failure: 0x%x\n", data);
		return -ERR_LINK_TRAINING_FAILURE;
	}

	/* Get Link Bandwidth */
	data = readl(&base->link_bw_set);

	dp->link_train.link_rate = data;

	data = readl(&base->lane_count_set);
	dp->link_train.lane_count = data;

	return 0;
}

/*
 * Initialize DP display
 * param node		DP node
 * return		Initialization status
 */
static int dp_main_init(int node)
{
	int ret;
	struct s5p_dp_device *dp = &dp_device;
	fdt_addr_t addr;
	struct exynos5_dp *base;

	addr = fdtdec_get_addr(gd->fdt_blob, node, "reg");
	if (addr == FDT_ADDR_T_NONE) {
		debug("%s: Missing dp-base\n", __func__);
		return -ERR_MISSING_DP_BASE;
	}
	dp->base = (struct exynos5_dp *)addr;
	dp->video_info = &smdk5250_dp_config;

	clock_init_dp_clock();

	power_enable_dp_phy();
	ret = s5p_dp_init_dp(dp);
	if (ret) {
		debug("%s: Could not initialize dp\n", __func__);
		return ret;
	}

	ret = s5p_dp_hw_link_training(dp, dp->video_info->lane_count,
				dp->video_info->link_rate);
	if (ret) {
		debug("unable to do link train\n");
		return ret;
	}
	/* Minimum delay after H/w Link training */
	mdelay(1);

	ret = s5p_dp_enable_scramble(dp);
	if (ret) {
		debug("unable to set scramble mode\n");
		return ret;
	}

	ret = s5p_dp_enable_rx_to_enhanced_mode(dp);
	if (ret) {
		debug("unable to set enhanced mode\n");
		return ret;
	}


	base = dp->base;
	/* Enable enhanced mode */
	setbits_le32(&base->sys_ctl_4, ENHANCED);

	writel(dp->video_info->lane_count, &base->lane_count_set);
	writel(dp->video_info->link_rate, &base->link_bw_set);

	s5p_dp_init_video(dp);
	ret = s5p_dp_config_video(dp, dp->video_info);
	if (ret) {
		debug("unable to config video\n");
		return ret;
	}

	return 0;
}

/*
 * Fill LCD timing data for DP or MIPI
 * param node	DP/FIMD node
 * return	(struct exynos5_fimd_panel *) LCD timing data
 */
static struct exynos5_fimd_panel *fill_panel_data(int node)
{
	const char *interface;
	int val;

	interface = fdt_getprop(gd->fdt_blob, node, "samsung,interface", NULL);
	val = interface && strcmp(interface, "edp");

	global_panel_data[0].xres = panel_info.vl_col;
	global_panel_data[0].yres = panel_info.vl_row;
	global_panel_data[1].xres = panel_info.vl_col;
	global_panel_data[1].yres = panel_info.vl_row;

	if (!val)
		/* Display I/F is eDP */
		return &global_panel_data[0];
	else
		/* Display I/F is MIPI */
		return &global_panel_data[1];

}

/**
 * Init the LCD controller
 *
 * @param lcdbase	Base address of LCD frame buffer
 * @return 0 if ok, -ve error code on error
 */
static int init_lcd_controller(void *lcdbase)
{
	struct exynos5_fimd_panel *panel_data;
	int node;

	/* Get the node from FDT for DP */
	node = fdtdec_next_compatible(gd->fdt_blob, 0,
					COMPAT_SAMSUNG_EXYNOS_DP);
	if (node < 0) {
		debug("EXYNOS_DP: No node for dp in device tree\n");
		return -ERR_NO_FDT_NODE;
	}

	pwm_init(0, MUX_DIV_2, 0);

	panel_data = fill_panel_data(node);

	if (panel_data->is_mipi)
		mipi_init();

	fimd_bypass();
	fb_init(lcdbase, panel_data);

	if (panel_data->is_dp) {
		int err;

		err = dp_main_init(node);
		if (err) {
			debug("DP initialization failed\n");
			return err;
		}
	}

	return 0;
}

void lcd_ctrl_init(void *lcdbase)
{
	int ret;

	/* We can't return an error, so for now print it */
	ret = init_lcd_controller(lcdbase);
	if (ret)
		printf("LCD init error %d\n", ret);

	/* Enable flushing after LCD writes if requested */
	lcd_set_flush_dcache(1);
}
