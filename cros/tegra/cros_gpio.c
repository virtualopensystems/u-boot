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
#include <asm/arch/tegra2.h>
#include <asm/global_data.h>
#include <cros/common.h>
#include <cros/cros_gpio.h>

#define PREFIX "cros_gpio: "

DECLARE_GLOBAL_DATA_PTR;

static char *gpio_name[CROS_GPIO_MAX_GPIO] = {
	"write-protect-switch",
	"recovery-switch",
	"developer-switch",
	"lid-switch",
	"power-switch",
};

static int g_config_node = -1;
static unsigned long g_valid_time;

int misc_init_r(void)
{
	struct fdt_gpio_state gs;
	int i, config_node;

	config_node = fdt_path_offset(gd->fdt_blob, "/config");
	if (config_node < 0)
		return -1;

	for (i = 0; i < CROS_GPIO_MAX_GPIO; i++) {
		if (fdtdec_decode_gpio(gd->fdt_blob, config_node, gpio_name[i],
				&gs))
			return -1;
		fdtdec_setup_gpio(&gs);
		if (fdt_gpio_isvalid(&gs))
			gpio_direction_input(gs.gpio);
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
	g_valid_time = timer_get_us() + 10;
	g_config_node = config_node;

	return 0;
}

int cros_gpio_fetch(enum cros_gpio_index index, cros_gpio_t *gpio)
{
	struct fdt_gpio_state gs;
	int p;

	assert(g_config_node >= 0);
	assert(index >= 0 && index < CROS_GPIO_MAX_GPIO);

	if (fdtdec_decode_gpio(gd->fdt_blob, g_config_node, gpio_name[index],
			&gs)) {
		VBDEBUG(PREFIX "fail to decode gpio: %d\n", index);
		return -1;
	}

	gpio->index = index;
	gpio->port = gs.gpio;
	gpio->polarity = (gs.flags & FDT_GPIO_ACTIVE_LOW) ?
		CROS_GPIO_ACTIVE_LOW : CROS_GPIO_ACTIVE_HIGH;
	p = (gpio->polarity == CROS_GPIO_ACTIVE_HIGH) ? 0 : 1;

	/* We can only read GPIO after g_valid_time */
	while (timer_get_us() < g_valid_time)
		udelay(10);

	gpio->value = p ^ gpio_get_value(gpio->port);

	return 0;
}

int cros_gpio_dump(cros_gpio_t *gpio)
{
#ifdef VBOOT_DEBUG
	const char const *name[CROS_GPIO_MAX_GPIO] = {
		"wpsw", "recsw", "devsw", "lidsw", "pwrsw"
	};
	int index = gpio->index;

	if (index < 0 || index >= CROS_GPIO_MAX_GPIO) {
		VBDEBUG(PREFIX "index out of range: %d\n", index);
		return -1;
	}

	VBDEBUG(PREFIX "%-6s: port=%3d, polarity=%d, value=%d\n",
			name[gpio->index],
			gpio->port, gpio->polarity, gpio->value);
#endif
	return 0;
}
