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
#include <cros/common.h>
#include <cros/vboot_flag.h>
#include <vboot_api.h>

uint32_t VbExIsShutdownRequested(void)
{
	cros_gpio_t lidsw, pwrsw;

	/* if lid is NOT OPEN */
	if (!cros_gpio_fetch(CROS_GPIO_LIDSW, &lidsw) && !lidsw.value) {
		VBDEBUG("Lid-closed is detected.\n");
		return 1;
	}
	/* if power switch is pressed */
	if (!cros_gpio_fetch(CROS_GPIO_PWRSW, &pwrsw) && pwrsw.value) {
		VBDEBUG("Power-key-pressed is detected.\n");
		return 1;
	}
	/*
	 * Either the gpios don't exist, or the lid is up and and power button
	 * is not pressed. No-Shutdown-Requested.
	 */
	return 0;
}
