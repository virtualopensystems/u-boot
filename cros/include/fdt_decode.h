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

#include <cros/fmap.h>

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

/**
 * Look up a property in chromeos-config which contains a memory region
 * address and size. Then return a pointer to this address. if the address
 * is zero, it is allocated with malloc() instead.
 *
 * The property must hold one address with a length. This is only tested on
 * 32-bit machines.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param size		returns size of region
 * @return pointer to region, or NULL if property not found/malloc failed
 */
void *fdt_decode_chromeos_alloc_region(const void *blob,
		const char *prop_name, size_t *size);

/**
 * Retrieve the MRC cache base address from the FMAP section of the device
 * tree.
 *
 * @param blob		FDT blob
 * @param fme		pointer to the return value (offset and length are
 *			saved in the structure)
 */
int fdt_get_mrc_cache_base(const char *blob, struct fmap_entry *fme);

#endif /* CHROMEOS_FDT_DECODE_H_ */
