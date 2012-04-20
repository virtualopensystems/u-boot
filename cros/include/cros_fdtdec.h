/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CROS_FDTDEC_H_
#define CROS_FDTDEC_H_

#include <fdtdec.h>
#include <cros/fmap.h>

/* Decode Chrome OS specific configuration from fdt */

int cros_fdtdec_flashmap(const void *fdt, struct twostop_fmap *config);

/**
 * Return offset of /chromeos-config node
 *
 * @param blob	FDT blob
 * @return the offset or -FDT_ERR_NOTFOUND if not found
 */
int cros_fdtdec_config_node(const void *blob);

/**
 * This checks whether a property exists.
 *
 * @param fdt	FDT blob to use
 * @param name	The path and name to the property in question
 * @return non-zero if the property exists, zero if it does not exist.
 */
int cros_fdtdec_config_has_prop(const void *fdt, const char *name);

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
void *cros_fdtdec_alloc_region(const void *blob,
		const char *prop_name, size_t *size);

/**
 * Retrieve the MRC cache base address from the FMAP section of the device
 * tree.
 *
 * @param blob		FDT blob
 * @param fme		pointer to the return value (offset and length are
 *			saved in the structure)
 */
int cros_fdtdec_mrc_cache_base(const char *blob, struct fmap_entry *fme);

/**
 * Returns information from the FDT about memory for a given root
 *
 * @param blob          FDT blob to use
 * @param name          Root name of alias to search for
 * @param config        structure to use to return information
 */
int cros_fdtdec_memory(const void *blob, const char *name,
		struct fdt_memory *config);

#endif /* CROS_FDTDEC_H_ */
