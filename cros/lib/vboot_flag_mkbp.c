/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of vboot flag accessor from communicating with EC */

#include <common.h>
#include <mkbp.h>
#include <cros/common.h>
#include <cros/vboot_flag.h>

static int vboot_flag_fetch_mkbp(enum vboot_flag_id id,
				 struct vboot_flag_context *context,
				 struct vboot_flag_details *details)
{
	struct mkbp_dev *dev;
	struct ec_response_mkbp_info info;

	dev = board_get_mkbp_dev();
	if (!dev) {
		VBDEBUG("%s: no mkbp device\n", __func__);
		return -1;
	}

	if (mkbp_info(dev, &info)) {
		VBDEBUG("Could not read KBC info\n");
		return -1;
	}

	/* TODO(waihong): So far only support recovery flag */
	switch (id) {
	case VBOOT_FLAG_RECOVERY:
		details->value = (info.switches &
				  EC_SWITCH_KEYBOARD_RECOVERY) ? 1 : 0;
		break;
	default:
		VBDEBUG("the flag is not supported reading from ec: %s\n",
			vboot_flag_node_name(id));
		return -1;
	}
	details->port = 0;
	details->active_high = 0;

	return 0;
}

struct vboot_flag_driver vboot_flag_driver_mkbp = {
	.type	= COMPAT_GOOGLE_CONST_FLAG,
	.setup	= NULL,
	.fetch	= vboot_flag_fetch_mkbp,
};
