/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
#include <lcd.h>

#include <asm/system.h>
#include <asm/gpio.h>

#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pwfm.h>
#include <asm/arch/display.h>
#include <asm/arch/timer.h>

DECLARE_GLOBAL_DATA_PTR;

/* These are the stages we go throuh in enabling the LCD */
enum stage_t {
	STAGE_START,
	STAGE_LVDS,
	STAGE_BACKLIGHT_VDD,
	STAGE_PWFM,
	STAGE_BACKLIGHT_EN,
	STAGE_DONE,
};

static enum stage_t stage;	/* Current stage we are at */
static unsigned long timer_next; /* Time we can move onto next stage */
static struct fdt_lcd config;	/* Our LCD config, set up in handle_stage() */

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= 4,		/* 2^4 = 16 bpp */
};

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

short console_col;
short console_row;

vidinfo_t panel_info = {
	/* Insert a value here so that we don't end up in the BSS */
	.vl_col = -1,
};

char lcd_cursor_enabled;

ushort lcd_cursor_width;
ushort lcd_cursor_height;

#ifndef CONFIG_OF_CONTROL
#error "You must enable CONFIG_OF_CONTROL to get Tegra LCD support"
#endif

void lcd_cursor_size(ushort width, ushort height)
{
	lcd_cursor_width = width;
	lcd_cursor_height = height;
}

void lcd_toggle_cursor(void)
{
	ushort x, y;
	uchar *dest;
	ushort row;

	x = console_col * lcd_cursor_width;
	y = console_row * lcd_cursor_height;
	dest = (uchar *)(lcd_base + y * lcd_line_length + x * (1 << LCD_BPP) /
			8);

	for (row = 0; row < lcd_cursor_height; ++row, dest += lcd_line_length) {
		ushort *d = (ushort *)dest;
		ushort color;
		int i;

		for (i = 0; i < lcd_cursor_width; ++i) {
			color = *d;
			color ^= lcd_color_fg;
			*d = color;
			++d;
		}
	}
}

void lcd_cursor_on(void)
{
	lcd_cursor_enabled = 1;
	lcd_toggle_cursor();
}
void lcd_cursor_off(void)
{
	lcd_cursor_enabled = 0;
	lcd_toggle_cursor();
}

char lcd_is_cursor_enabled(void)
{
	return lcd_cursor_enabled;
}

static void update_panel_size(struct fdt_lcd *config)
{
	panel_info.vl_col = config->width;
	panel_info.vl_row = config->height;
	panel_info.vl_bpix = config->log2_bpp;
}

/*
 *  Main init function called by lcd driver.
 *  Inits and then prints test pattern if required.
 */

void lcd_ctrl_init(void *lcdbase)
{
	int line_length, size;

	/*
	 * The framebuffer address should be specified in the device tree.
	 * This FDT value should be the same as the one defined in Linux kernel;
	 * otherwise, it causes screen flicker. The FDT value overrides the
	 * framebuffer allocated at the top of memory by board_init_f().
	 *
	 * If the framebuffer address is not defined in the FDT, falls back to
	 * use the address allocated by board_init_f().
	 */
	if (config.frame_buffer != FDT_ADDR_T_NONE) {
		gd->fb_base = config.frame_buffer;
		lcd_base = (void *)(gd->fb_base);
	} else {
		config.frame_buffer = (u32)lcd_base;
	}

	/* Make sure that we can acommodate the selected LCD */
	assert(config.width <= LCD_MAX_WIDTH);
	assert(config.height <= LCD_MAX_HEIGHT);
	assert(config.log2_bpp <= LCD_MAX_LOG2_BPP);
	if (config.width <= LCD_MAX_WIDTH && config.height <= LCD_MAX_HEIGHT &&
			config.log2_bpp <= LCD_MAX_LOG2_BPP)
		update_panel_size(&config);
	size = lcd_get_size(&line_length);

	/* Initialize the Tegra display controller */
	if (tegra2_display_register(&config)) {
		printf("%s: Failed to register with display driver\n",
		       __func__);
		return;
	}

	debug("LCD frame buffer at %p\n", lcd_base);
}

ulong calc_fbsize(void)
{
	return (panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

void tegra_lcd_early_init(const void *blob)
{
	/*
	 * Go with the maximum size for now. We will fix this up after
	 * relocation. These values are only used for memory alocation.
	 */
	panel_info.vl_col = LCD_MAX_WIDTH;
	panel_info.vl_row = LCD_MAX_HEIGHT;
	panel_info.vl_bpix = LCD_MAX_LOG2_BPP;
}

int fdt_decode_lcd(const void *blob, struct fdt_lcd *config)
{
	int err, bpp, bit;
	int lcd_node, display_node;

	display_node = fdtdec_next_compatible(gd->fdt_blob, 0,
				      COMPAT_NVIDIA_TEGRA20_DISPLAY);
	if (display_node < 0)
		return display_node;

	lcd_node = fdt_next_node(blob, display_node, NULL);
	if (lcd_node < 0)
		return lcd_node;

	config->width = fdtdec_get_int(blob, lcd_node, "nvidia,width", -1);
	config->height = fdtdec_get_int(blob, lcd_node, "nvidia,height", -1);
	bpp = fdtdec_get_int(blob, lcd_node, "nvidia,bits-per-pixel", -1);
	bit = ffs(bpp) - 1;
	if (bpp == (1 << bit))
		config->log2_bpp = bit;
	else
		config->log2_bpp = bpp;
	config->bpp = bpp;
	config->pixel_clock = fdtdec_get_int(blob, lcd_node,
					     "nvidia,pixel-clock", 0);
	if (!config->pixel_clock || bpp == -1 ||
			config->width == -1 || config->height == -1)
		return -FDT_ERR_NOTFOUND;
	err = fdtdec_get_int_array(blob, lcd_node, "nvidia,horiz-timing",
			config->horiz_timing, FDT_LCD_TIMING_COUNT);
	if (!err)
		err = fdtdec_get_int_array(blob, lcd_node, "nvidia,vert-timing",
				config->vert_timing, FDT_LCD_TIMING_COUNT);
	if (err)
		return err;

	config->disp = (struct disp_ctlr *)fdtdec_get_addr(blob,
						display_node, "reg");
	config->pwfm = fdtdec_lookup_phandle_reg(blob, display_node,
						 "nvidia,pwfm");
	if ((fdt_addr_t)config->disp == FDT_ADDR_T_NONE ||
			(fdt_addr_t)config->pwfm == FDT_ADDR_T_NONE)
		return -1;
	config->cache_type = fdtdec_get_int(blob, display_node,
					    "nvidia,cache-type",
					    FDT_LCD_CACHE_WRITE_BACK_FLUSH);
	config->frame_buffer = fdtdec_get_addr(blob, display_node,
					       "nvidia,frame-buffer");

	err |= fdtdec_decode_gpio(blob, display_node,
				  "nvidia,backlight-enable-gpios",
				  &config->backlight_en);
	err |= fdtdec_decode_gpio(blob, display_node,
			"nvidia,lvds-shutdown-gpios", &config->lvds_shutdown);
	fdtdec_decode_gpio(blob, display_node, "nvidia,backlight-vdd-gpios",
			   &config->backlight_vdd);
	err |= fdtdec_decode_gpio(blob, display_node, "nvidia,panel-vdd-gpios",
				  &config->panel_vdd);
	if (err)
		return -FDT_ERR_NOTFOUND;

	return fdtdec_get_int_array(blob, display_node, "nvidia,panel-timings",
			config->panel_timings, FDT_LCD_TIMINGS);
}

/**
 * Handle the next stage of device init
 */
static int handle_stage(const void *blob)
{
	debug("%s: stage %d\n", __func__, stage);

	/* do the things for this stage */
	switch (stage) {
	case STAGE_START:
		/* get panel details */
		if (fdt_decode_lcd(blob, &config)) {
			printf("No LCD information in device tree\n");
			return -1;
		}

		/*
		 * It is possible that the FDT has requested that the LCD be
		 * disabled. We currently don't support this. It would require
		 * changes to U-Boot LCD subsystem to have LCD support
		 * compiled in but not used. An easier option might be to
		 * still have a frame buffer, but leave the backlight off and
		 * remove all mention of lcd in the stdout environment
		 * variable.
		 */

		funcmux_select(PERIPH_ID_DISP1, FUNCMUX_DEFAULT);

		fdtdec_setup_gpio(&config.panel_vdd);
		fdtdec_setup_gpio(&config.lvds_shutdown);
		fdtdec_setup_gpio(&config.backlight_vdd);
		fdtdec_setup_gpio(&config.backlight_en);

		/*
		 * TODO: If fdt includes output flag we can omit this code
		 * since fdtdec_setup_gpio will do it for us.
		 */
		gpio_direction_output(config.panel_vdd.gpio, 1);
		gpio_direction_output(config.lvds_shutdown.gpio, 0);
		gpio_direction_output(config.backlight_vdd.gpio, 0);
		gpio_direction_output(config.backlight_en.gpio, 0);
		break;

	case STAGE_LVDS:
		gpio_set_value(config.lvds_shutdown.gpio, 1);
		break;

	case STAGE_BACKLIGHT_VDD:
		if (fdt_gpio_isvalid(&config.backlight_vdd))
			gpio_set_value(config.backlight_vdd.gpio, 1);
		break;

	case STAGE_PWFM:
		/* Enable PWM at 15/16 high, 32768 Hz with divider 1 */
		pinmux_set_func(PINGRP_GPU, PMUX_FUNC_PWM);
		pinmux_tristate_disable(PINGRP_GPU);
		pwfm_enable(config.pwfm, 32768, 0xdf, 1);
		break;

	case STAGE_BACKLIGHT_EN:
		gpio_set_value(config.backlight_en.gpio, 1);
		break;

	case STAGE_DONE:
		break;
	}

	/* set up timer for next stage */
	timer_next = timer_get_us() + config.panel_timings[stage] * 1000;

	/* move to next stage */
	stage++;
	return 0;
}

int tegra_lcd_check_next_stage(const void *blob, int wait)
{
	if (stage == STAGE_DONE)
		return 0;

	do {
		/* wait if we need to */
		debug("%s: stage %d\n", __func__, stage);
		if (stage != STAGE_START) {
			int delay = timer_next - timer_get_us();

			if (delay > 0) {
				if (wait)
					udelay(delay);
				else
					return 0;
			}
		}

		if (handle_stage(blob))
			return -1;
	} while (wait && stage != STAGE_DONE);
	return 0;
}

void lcd_enable(void)
{
	/*
	 * Backlight and power init will be done separately in
	 * tegra_lcd_check_next_stage(), which should be called in
	 * board_late_init().
	 *
	 * U-Boot code supports only colour depth, selected at compile time.
	 * The device tree setting should match this. Otherwise the display
	 * will not look right, and U-Boot may crash.
	 */
	if (config.log2_bpp != LCD_BPP) {
		printf("%s: Error: LCD depth configured in FDT (%d = %dbpp)"
			" must match setting of LCD_BPP (%d)\n", __func__,
		       config.log2_bpp, config.bpp, LCD_BPP);
	}
}
