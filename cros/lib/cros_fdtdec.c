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
#include <libfdt.h>
#include <cros/common.h>
#include <cros/cros_fdtdec.h>
#include <cros/fmap.h>
#include <fdtdec.h>
#include <linux/string.h>
#include <malloc.h>

static int relpath_offset(const void *blob, int offset, const char *in_path)
{
	const char *path = in_path;
	const char *sep;

	/* skip leading '/' */
	while (*path == '/')
		path++;

	for (sep = path; *sep; path = sep + 1) {
		/* this is equivalent to strchrnul() */
		sep = strchr(path, '/');
		if (!sep)
			sep = path + strlen(path);

		offset = fdt_subnode_offset_namelen(blob, offset, path,
				sep - path);
		if (offset < 0) {
			VBDEBUG("Node '%s' is missing\n", in_path);
			return offset;
		}
	}

	return offset;
}

static int decode_fmap_entry(const void *blob, int offset, const char *base,
		const char *name, struct fmap_entry *entry)
{
	char path[50];
	int length;
	uint32_t *property;

	/* Form the node to look up as <base>-<name> */
	assert(strlen(base) + strlen(name) + 1 < sizeof(path));
	strcpy(path, base);
	strcat(path, "-");
	strcat(path, name);

	offset = relpath_offset(blob, offset, path);
	if (offset < 0)
		return offset;
	property = (uint32_t *)fdt_getprop(blob, offset, "reg", &length);
	if (!property) {
		VBDEBUG("Node '%s' is missing property '%s'\n", path, "reg");
		return -FDT_ERR_MISSING;
	}
	entry->offset = fdt32_to_cpu(property[0]);
	entry->length = fdt32_to_cpu(property[1]);

	return 0;
}

static int decode_block_lba(const void *blob, int offset, const char *path,
		uint64_t *out)
{
	int length;
	uint32_t *property;

	offset = relpath_offset(blob, offset, path);
	if (offset < 0)
		return offset;

	property = (uint32_t *)fdt_getprop(blob, offset, "block-lba", &length);
	if (!property) {
		VBDEBUG("failed to load LBA '%s/block-lba'\n", path);
		return -FDT_ERR_MISSING;
	}
	*out = fdt32_to_cpu(*property);
	return 0;
}

int decode_firmware_entry(const char *blob, int fmap_offset, const char *name,
		struct fmap_firmware_entry *entry)
{
	int err;

	err = decode_fmap_entry(blob, fmap_offset, name, "boot", &entry->boot);
	err |= decode_fmap_entry(blob, fmap_offset, name, "vblock",
			&entry->vblock);
	err |= decode_fmap_entry(blob, fmap_offset, name, "firmware-id",
			&entry->firmware_id);
	err |= decode_block_lba(blob, fmap_offset, name, &entry->block_lba);
	return err;
}

int cros_fdtdec_config_node(const void *blob)
{
	int node = fdt_path_offset(blob, "/chromeos-config");

	if (node < 0)
		VBDEBUG("failed to find /chromeos-config: %d\n", node);

	return node;
}

int cros_fdtdec_mrc_cache_base(const char *blob, struct fmap_entry *fme)
{
	int fmap_offset;

	fmap_offset = fdt_node_offset_by_compatible(blob, -1,
			"chromeos,flashmap");
	if (fmap_offset < 0) {
		VBDEBUG("chromeos,flashmap node is missing\n");
		return fmap_offset;
	}

	return decode_fmap_entry(blob, fmap_offset, "rw", "mrc-cache", fme);
}

int cros_fdtdec_flashmap(const void *blob, struct twostop_fmap *config)
{
	int fmap_offset;
	int err;
	uint32_t *property;
	int length;

	fmap_offset = fdt_node_offset_by_compatible(blob, -1,
			"chromeos,flashmap");
	if (fmap_offset < 0) {
		VBDEBUG("chromeos,flashmap node is missing\n");
		return fmap_offset;
	}

	property = (uint32_t *)fdt_getprop(blob, fmap_offset, "reg", &length);
	if (!property || (length != 8)) {
		VBDEBUG("Flashmap node missing the `reg' property\n");
		return -FDT_ERR_MISSING;
	}

	config->flash_base = fdt32_to_cpu(property[0]);

	err = decode_firmware_entry(blob, fmap_offset, "rw-a",
			&config->readwrite_a);
	err |= decode_firmware_entry(blob, fmap_offset, "rw-b",
			&config->readwrite_b);

	err |= decode_fmap_entry(blob, fmap_offset, "ro", "fmap",
			&config->readonly.fmap); \
	err |= decode_fmap_entry(blob, fmap_offset, "ro", "gbb",
			&config->readonly.gbb); \
	err |= decode_fmap_entry(blob, fmap_offset, "ro", "firmware-id",
			&config->readonly.firmware_id);

	return 0;
}

int cros_fdtdec_config_has_prop(const void *blob, const char *name)
{
	int nodeoffset = cros_fdtdec_config_node(blob);

	return nodeoffset >= 0 &&
		fdt_get_property(blob, nodeoffset, name, NULL) != NULL;
}

void *cros_fdtdec_alloc_region(const void *blob,
		const char *prop_name, size_t *size)
{
	int node = cros_fdtdec_config_node(blob);
	void *ptr;

	if (node < 0)
		return NULL;

	if (fdtdec_decode_region(blob, node, prop_name, &ptr, size)) {
		VBDEBUG("failed to find %s in /chromeos-config'\n", prop_name);
		return NULL;
	}

	if (!ptr)
		ptr = malloc(*size);
	if (!ptr) {
		VBDEBUG("failed to alloc %d bytes for %s'\n", *size, prop_name);
	}
	return ptr;
}

int cros_fdtdec_memory(const void *blob, const char *name,
		struct fdt_memory *config)
{
	int node, len;
	const fdt_addr_t *cell;

	node = fdt_path_offset(blob, name);
	if (node < 0)
		return node;

	cell = fdt_getprop(blob, node, "reg", &len);
	if (cell && len == sizeof(fdt_addr_t) * 2) {
		config->start = fdt_addr_to_cpu(cell[0]);
		config->end = fdt_addr_to_cpu(cell[1]);
	} else
		return -FDT_ERR_BADLAYOUT;

	return 0;
}
