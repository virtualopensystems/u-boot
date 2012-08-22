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
#include <asm/gpio.h>

#include "asm/sandbox-api.h"

/* Flags for each GPIO */
#define GPIOF_OUTPUT	(1 << 0)	/* Currently set as an output */
#define GPIOF_HIGH	(1 << 1)	/* Currently set high */
#define GPIOF_RESERVED	(1 << 2)	/* Is in use / requested */

static const char *gpio_name[CONFIG_SANDBOX_GPIO_COUNT];

/* Access routines for GPIOs */
static u8 *get_gpio_address(unsigned gp)
{
	if (gp >= CONFIG_SANDBOX_GPIO_COUNT) {
		static u8 invalid_flags;
		printf("sandbox_gpio: error: invalid gpio %u\n", gp);
		return &invalid_flags;
	}

	return sandbox_region_address(SML_GPIO) + gp;
}

static int get_gpio_flag(unsigned gp, int flag)
{
	return (*get_gpio_address(gp) & flag) != 0;
}

static int set_gpio_flag(unsigned gp, int flag, int value)
{
	u8 *gpio = get_gpio_address(gp);

	if (value)
		*gpio |= flag;
	else
		*gpio &= ~flag;

	return 0;
}

static int check_reserved(unsigned gpio, const char *func)
{
	if (!get_gpio_flag(gpio, GPIOF_RESERVED)) {
		printf("sandbox_gpio: %s: error: gpio %u not reserved\n",
			func, gpio);
		return -1;
	}

	return 0;
}

/*
 * Back-channel sandbox-internal-only access to GPIO state
 */

int sandbox_gpio_get_value(unsigned gp)
{
	if (get_gpio_flag(gp, GPIOF_OUTPUT))
		debug("sandbox_gpio: get_value on output gpio %u\n", gp);
	return get_gpio_flag(gp, GPIOF_HIGH);
}

int sandbox_gpio_set_value(unsigned gp, int value)
{
	return set_gpio_flag(gp, GPIOF_HIGH, value);
}

int sandbox_gpio_get_direction(unsigned gp)
{
	return get_gpio_flag(gp, GPIOF_OUTPUT);
}

int sandbox_gpio_set_direction(unsigned gp, int output)
{
	return set_gpio_flag(gp, GPIOF_OUTPUT, output);
}

/*
 * These functions implement the public interface within U-Boot
 */

/* set GPIO port 'gp' as an input */
int gpio_direction_input(unsigned gp)
{
	debug("%s: gp:%u\n", __func__, gp);

	if (check_reserved(gp, __func__))
		return -1;

	return sandbox_gpio_set_direction(gp, 0);
}

/* set GPIO port 'gp' as an output, with polarity 'value' */
int gpio_direction_output(unsigned gp, int value)
{
	debug("%s: gp:%u, value = %d\n", __func__, gp, value);

	if (check_reserved(gp, __func__))
		return -1;

	return sandbox_gpio_set_direction(gp, 1) |
		sandbox_gpio_set_value(gp, value);
}

/* read GPIO IN value of port 'gp' */
int gpio_get_value(unsigned gp)
{
	debug("%s: gp:%u\n", __func__, gp);

	if (check_reserved(gp, __func__))
		return -1;

	return sandbox_gpio_get_value(gp);
}

/* write GPIO OUT value to port 'gp' */
int gpio_set_value(unsigned gp, int value)
{
	debug("%s: gp:%u, value = %d\n", __func__, gp, value);

	if (check_reserved(gp, __func__))
		return -1;

	if (!sandbox_gpio_get_direction(gp)) {
		printf("sandbox_gpio: error: set_value on input gpio %u\n", gp);
		return -1;
	}

	return sandbox_gpio_set_value(gp, value);
}

int gpio_request(unsigned gp, const char *label)
{
	debug("%s: gp:%u, label:%s\n", __func__, gp, label);

	if (gp >= CONFIG_SANDBOX_GPIO_COUNT) {
		printf("sandbox_gpio: error: invalid gpio %u\n", gp);
		return -1;
	}

	if (get_gpio_flag(gp, GPIOF_RESERVED)) {
		printf("sandbox_gpio: error: gpio %u already reserved\n", gp);
		return -1;
	}

	gpio_name[gp] = label;
	return set_gpio_flag(gp, GPIOF_RESERVED, 1);
}

int gpio_free(unsigned gp)
{
	debug("%s: gp:%u\n", __func__, gp);

	if (check_reserved(gp, __func__))
		return -1;

	gpio_name[gp] = NULL;
	return set_gpio_flag(gp, GPIOF_RESERVED, 0);
}

/* Display GPIO information */
void gpio_info(void)
{
	unsigned gpio;

	puts("Sandbox GPIOs\n");

	for (gpio = 0; gpio < CONFIG_SANDBOX_GPIO_COUNT; ++gpio) {
		const char *label = gpio_name[gpio];

		printf("%4d: %s: %d [%c] %s\n",
			gpio,
			sandbox_gpio_get_direction(gpio) ? "out" : " in",
			sandbox_gpio_get_value(gpio),
			get_gpio_flag(gpio, GPIOF_RESERVED) ? 'x' : ' ',
			label ? label : "");
	}
}
