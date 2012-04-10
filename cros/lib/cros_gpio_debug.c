/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <cros/common.h>
#include <cros/cros_gpio.h>

int cros_gpio_dump(cros_gpio_t *gpio)
{
	const char const *name[CROS_GPIO_MAX_GPIO] = {
		"wpsw", "recsw", "devsw", "lidsw", "pwrsw"
	};
	int index = gpio->index;

	if (index < 0 || index >= CROS_GPIO_MAX_GPIO) {
		VBDEBUG("index out of range: %d\n", index);
		return -1;
	}

	VBDEBUG("%-6s: port=%3d, polarity=%d, value=%d\n",
			name[gpio->index],
			gpio->port, gpio->polarity, gpio->value);
	return 0;
}
