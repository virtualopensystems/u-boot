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
#include <vboot_api.h>

int VbExTrustEC(void)
{
	struct vboot_flag_details gpio_ec_in_rw;
	int okay;

	/* If we don't have a valid GPIO to read, we can't trust it. */
	if (vboot_flag_fetch(VBOOT_FLAG_EC_IN_RW, &gpio_ec_in_rw)) {
		VBDEBUG("can't find GPIO to read, returning 0\n");
		return 0;
	}

	/* We only trust it if it's NOT in its RW firmware. */
	okay = !gpio_ec_in_rw.value;

	VBDEBUG("port=%d value=%d, returning %d\n",
		gpio_ec_in_rw.port, gpio_ec_in_rw.value, okay);

	return okay;
}
