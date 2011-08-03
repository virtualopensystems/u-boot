/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* Implementation of per-board fmap accessor functions */

#include <chromeos/fmap.h>
#include <common.h>
#include <chromeos/fdt_decode.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int
decode_twostop_fmap(struct twostop_fmap *fmap)
{
	return fdt_decode_twostop_fmap(gd->blob, fmap);
}
