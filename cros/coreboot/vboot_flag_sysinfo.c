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
#include <asm/arch-coreboot/ipchecksum.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <asm/arch-coreboot/tables.h>
#include <cros/common.h>
#include <cros/vboot_flag.h>

DECLARE_GLOBAL_DATA_PTR;

int cros_gpio_init(void)
{
	return 0;
}

int cros_gpio_fetch(enum cros_gpio_index index, cros_gpio_t *gpio)
{
	const char const *name[CROS_GPIO_MAX_GPIO] = {
		"write protect",
		"recovery",
		"developer",
		"lid",
		"power",
	};

	int i;

	if (index < 0 || index >= CROS_GPIO_MAX_GPIO) {
		VBDEBUG("index out of range: %d\n", index);
		return -1;
	}

	for (i = 0; i < lib_sysinfo.num_gpios; i++) {
		int p;

		if (strncmp((char *)lib_sysinfo.gpios[i].name, name[index],
						GPIO_MAX_NAME_LENGTH))
			continue;

		/* Entry found */
		gpio->index = index;
		gpio->port = lib_sysinfo.gpios[i].port;
		gpio->polarity = lib_sysinfo.gpios[i].polarity;
		gpio->value = lib_sysinfo.gpios[i].value;

		p = (gpio->polarity == CROS_GPIO_ACTIVE_HIGH) ? 0 : 1;
		gpio->value = p ^ gpio->value;

		return 0;
	}

	VBDEBUG("failed to find gpio port\n");
	return -1;
}
