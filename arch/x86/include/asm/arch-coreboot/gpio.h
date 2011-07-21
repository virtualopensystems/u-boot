/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
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

#ifndef _I386_COREBOOT_GPIO_H_
#define _I386_COREBOOT_GPIO_H_

#include <common.h>

static inline
int gpio_direction_input(int gp)
{
	printf("gpio_direction_input used but not implemented.\n");
	return 0;
}

static inline
int gpio_direction_output(int gp, int value)
{
	printf("gpio_direction_output used but not implemented.\n");
	return 0;
}

static inline
int gpio_get_value(int gp)
{
	printf("gpio_get_value used but not implemented.\n");
	return 0;
}

static inline
void gpio_set_value(int gp, int value)
{
	printf("gpio_set_value used but not implemented.\n");
}

#endif	/* _I386_COREBOOT_GPIO_H_ */
