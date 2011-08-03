/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CHROMEOS_FDT_DECODE_H_
#define CHROMEOS_FDT_DECODE_H_

#include <chromeos/fmap.h>

/* Decode Chrome OS specific configuration from fdt */

int fdt_decode_twostop_fmap(const void *fdt, struct twostop_fmap *config);

/**
 * This checks whether a property exists.
 *
 * @param fdt	FDT blob to use
 * @param name	The path and name to the property in question
 * @return non-zero if the property exists, zero if it does not exist.
 */
int fdt_decode_chromeos_config_has_prop(const void *fdt, const char *name);

#endif /* CHROMEOS_FDT_DECODE_H_ */
