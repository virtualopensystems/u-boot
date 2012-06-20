/*
 * Copyright (C) 2012 The Chromium Authors
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
#include <mkbp.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mkbp_dev *mkbp_dev;	/* Pointer to mkbp device */

struct mkbp_dev *board_get_mkbp_dev(void)
{
	return mkbp_dev;
}

static int board_init_mkbp_devices(const void *blob)
{
#ifdef CONFIG_MKBP
	mkbp_dev = mkbp_init(gd->fdt_blob);
	if (!mkbp_dev) {
		debug("%s: cannot init mkbp device\n", __func__);
		return -1;
	}
#endif
	return 0;
}

int board_init(void)
{
	if (board_init_mkbp_devices(gd->fdt_blob))
		return -1;

	return 0;
}
