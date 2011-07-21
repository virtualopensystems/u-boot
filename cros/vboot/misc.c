/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <chromeos/common.h>
#include <chromeos/cros_gpio.h>
#include <vboot_api.h>

#define PREFIX			"misc: "

DECLARE_GLOBAL_DATA_PTR;

uint32_t VbExIsShutdownRequested(void)
{
	void *fdt_ptr = (void *)gd->blob;
	cros_gpio_t lidsw, pwrsw;

	if (cros_gpio_fetch(CROS_GPIO_LIDSW, fdt_ptr, &lidsw) ||
			cros_gpio_fetch(CROS_GPIO_PWRSW, fdt_ptr, &pwrsw)) {
		VBDEBUG(PREFIX "Failed to fetch GPIO!\n");
		/* still return 0, No-Shutdown-Requested */
		return 0;
	}

	/* if lid is NOT OPEN */
	if (!lidsw.value) {
		VBDEBUG(PREFIX "Lid-closed is detected.\n");
		return 1;
	}

	if (pwrsw.value) {
		VBDEBUG(PREFIX "Power-key-pressed is detected.\n");
		return 1;
	}

	return 0;
}
