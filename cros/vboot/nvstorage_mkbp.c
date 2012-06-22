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
#include <mkbp.h>
#include <cros/common.h>
#include <cros/nvstorage.h>

#include <vboot_api.h>

VbError_t nvstorage_read_mkbp(uint8_t *buf)
{
	struct mkbp_dev *dev;

	dev = board_get_mkbp_dev();
	if (!dev) {
		VBDEBUG("%s: no mkbp device\n", __func__);
		return 1;
	}

	if (mkbp_read_vbnvcontext(dev, buf))
		return 1;

	return VBERROR_SUCCESS;
}

VbError_t nvstorage_write_mkbp(const uint8_t *buf)
{
	struct mkbp_dev *dev;

	dev = board_get_mkbp_dev();
	if (!dev) {
		VBDEBUG("%s: no mkbp device\n", __func__);
		return 1;
	}

	if (mkbp_write_vbnvcontext(dev, buf))
		return 1;

	return VBERROR_SUCCESS;
}
