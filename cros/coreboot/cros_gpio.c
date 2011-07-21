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

#include <chromeos/cros_gpio.h>
#include <common.h>

int cros_gpio_fetch(enum cros_gpio_index index, const void *fdt,
		cros_gpio_t *gpio)
{
	printf("cros_gpio_fetch used but not implemented.\n");
	return 0;
}

int cros_gpio_dump(cros_gpio_t *gpio)
{
	printf("cros_gpio_dump used but not implemented.\n");
	return 0;
}
