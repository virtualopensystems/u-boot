/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of per-board GPIO accessor functions */

#include <common.h>
#include <fdtdec.h>
#include <asm-generic/gpio.h>
#include <asm/global_data.h>
#include <cros/common.h>
#include <cros/cros_gpio.h>

DECLARE_GLOBAL_DATA_PTR;

static char *gpio_name[CROS_GPIO_MAX_GPIO] = {
	"write-protect-switch",
	"recovery-switch",
	"developer-switch",
	"lid-switch",
	"power-switch",
};

static int config = -1;
static unsigned long valid_time;

/* Default empty implementation */
static int __cros_gpio_setup(enum cros_gpio_index index, int port)
{
	return 0;
}

int cros_gpio_setup(enum cros_gpio_index index, int port)
	__attribute__((weak, alias("__cros_gpio_setup")));

int cros_gpio_init(void)
{
	const void *blob = gd->fdt_blob;
	struct fdt_gpio_state gs;
	unsigned long delay_time;
	int i;

	config = fdt_path_offset(blob, "/chromeos-config");
	if (config < 0)
		return -1;

	for (i = 0; i < CROS_GPIO_MAX_GPIO; i++) {
		if (fdtdec_decode_gpio(blob, config, gpio_name[i], &gs)) {
			VBDEBUG("decoding GPIO failed: %s\n", gpio_name[i]);
			continue;
		}
		fdtdec_setup_gpio(&gs);
		if (fdt_gpio_isvalid(&gs)) {
			if (cros_gpio_setup(i, gs.gpio)) {
				VBDEBUG("setup failed: %s\n", gpio_name[i]);
				continue;
			}
			gpio_direction_input(gs.gpio);
		}
	}

	/*
	 * In theory we have to insert a delay here for charging the input
	 * gate capacitance. Consider a 200K ohms series resister and 10
	 * picofarads gate capacitance.
	 *
	 * RC time constant is
	 *     200 K ohms * 10 picofarads = 2 microseconds
	 *
	 * Then 10-90% rise time is
	 *     2 microseconds * 2.2 = 4.4 microseconds
	 *
	 * Thus, 10 microseconds gives us a 50% margin.
	 */
	delay_time = fdtdec_get_int(blob, config,
			"cros-gpio-input-charging-delay", 0);
	if (delay_time)
		valid_time = timer_get_us() + delay_time;

	return 0;
}

int cros_gpio_fetch(enum cros_gpio_index index, cros_gpio_t *gpio)
{
	struct fdt_gpio_state gs;
	int p;

	assert(config >= 0);
	assert(index >= 0 && index < CROS_GPIO_MAX_GPIO);

	if (fdtdec_decode_gpio(gd->fdt_blob, config, gpio_name[index], &gs)) {
		VBDEBUG("fail to decode gpio: %d\n", index);
		return -1;
	}

	gpio->index = index;
	gpio->port = gs.gpio;
	gpio->polarity = (gs.flags & FDT_GPIO_ACTIVE_LOW) ?
		CROS_GPIO_ACTIVE_LOW : CROS_GPIO_ACTIVE_HIGH;
	p = (gpio->polarity == CROS_GPIO_ACTIVE_HIGH) ? 0 : 1;

	if (valid_time) {
		/* We can only read GPIO after valid_time */
		while (timer_get_us() < valid_time)
			udelay(10);
	}

	gpio->value = p ^ gpio_get_value(gpio->port);

	return 0;
}
