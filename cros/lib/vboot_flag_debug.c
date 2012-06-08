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
#include <cros/vboot_flag.h>

#ifdef VBOOT_DEBUG
int cros_gpio_dump(cros_gpio_t *gpio)
{
	const char const *name[VBOOT_FLAG_MAX_FLAGS] = {
		"wpsw", "recsw", "devsw", "lidsw", "pwrsw"
	};
	int id = gpio->id;

	if (id < 0 || id >= VBOOT_FLAG_MAX_FLAGS) {
		VBDEBUG("id out of range: %d\n", id);
		return -1;
	}

	VBDEBUG("%-6s: port=%3d, polarity=%d, value=%d\n",
			name[gpio->id],
			gpio->port, gpio->polarity, gpio->value);
	return 0;
}
#else
int cros_gpio_dump(cros_gpio_t *gpio)
{
	return 0;
}
#endif
