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
#include <lcd.h>

DECLARE_GLOBAL_DATA_PTR;

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

short console_col;
short console_row;

vidinfo_t panel_info = {
	.vl_col = 640,
	.vl_row = 480,
	.vl_bpix = 8,
};

char lcd_cursor_enabled;	/* set initial value to false */

ushort lcd_cursor_width;
ushort lcd_cursor_height;

void lcd_cursor_size(ushort width, ushort height)
{
	lcd_cursor_width = width;
	lcd_cursor_height = height;
}

void lcd_toggle_cursor(void)
{
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

/*
 *  Main init function called by lcd driver.
 *  Inits and then prints test pattern if required.
 */

void lcd_ctrl_init(void *lcdbase)
{
}

ulong calc_fbsize(void)
{
	return (panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

void lcd_enable(void)
{
}
