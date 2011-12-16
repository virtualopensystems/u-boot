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
#include <gbb_header.h> /* for GoogleBinaryBlockHeader */
#include <chromeos/common.h>
#include <chromeos/crossystem_data.h>
#include <linux/string.h>

#ifdef CONFIG_OF_LIBFDT
#include <fdt_decode.h>
#include <fdt_support.h>
#include <libfdt.h>
#endif

#ifdef CONFIG_X86
#include <asm/ic/coreboot/sysinfo.h>
#endif

#define CROSSYSTEM_DATA_SIGNATURE "CHROMEOS"

/* This is used to keep bootstub and readwite main firmware in sync */
#define CROSSYSTEM_DATA_VERSION 1

#define PREFIX "crossystem_data: "

enum VdatFwIndex {
	VDAT_RW_A = 0,
	VDAT_RW_B = 1,
	VDAT_RECOVERY = 0xFF
};

enum BinfFwIndex {
	BINF_RECOVERY = 0,
	BINF_RW_A = 1,
	BINF_RW_B = 2
};

DECLARE_GLOBAL_DATA_PTR;

int crossystem_data_init(crossystem_data_t *cdata,
		cros_gpio_t *write_protect_switch,
		cros_gpio_t *recovery_switch,
		cros_gpio_t *developer_switch,
		uint32_t fmap_offset,
		uint8_t active_ec_firmware,
		uint8_t *hardware_id,
		uint8_t *readonly_firmware_id)
{
	VBDEBUG(PREFIX "crossystem data at %p\n", cdata);

	memset(cdata, '\0', sizeof(*cdata));

	cdata->total_size = sizeof(*cdata);
	cdata->version = CROSSYSTEM_DATA_VERSION;
	memcpy(cdata->signature, CROSSYSTEM_DATA_SIGNATURE,
			sizeof(CROSSYSTEM_DATA_SIGNATURE));

	cdata->boot_write_protect_switch = write_protect_switch->value;
	cdata->boot_recovery_switch = recovery_switch->value;
	cdata->boot_developer_switch = developer_switch->value;

	cdata->polarity_write_protect_switch = write_protect_switch->polarity;
	cdata->polarity_recovery_switch = recovery_switch->polarity;
	cdata->polarity_developer_switch = developer_switch->polarity;

	cdata->gpio_port_write_protect_switch = write_protect_switch->port;
	cdata->gpio_port_recovery_switch = recovery_switch->port;
	cdata->gpio_port_developer_switch = developer_switch->port;

	cdata->fmap_offset = fmap_offset;

	cdata->active_ec_firmware = active_ec_firmware;
	memcpy(cdata->hardware_id, hardware_id, sizeof(cdata->hardware_id));
	memcpy(cdata->readonly_firmware_id, readonly_firmware_id,
			sizeof(cdata->readonly_firmware_id));

#ifdef CONFIG_TEGRA2
	cdata->board.arm.nonvolatile_context_lba = CHROMEOS_VBNVCONTEXT_LBA;
	cdata->board.arm.nonvolatile_context_offset = 0;
	cdata->board.arm.nonvolatile_context_size = VBNV_BLOCK_SIZE;
#endif

	return 0;
}

int crossystem_data_check_integrity(crossystem_data_t *cdata)
{
	if (cdata->total_size != sizeof(*cdata)) {
		VBDEBUG(PREFIX "blob size mismatch: %08x != %08x\n",
				cdata->total_size, sizeof(*cdata));
		return 1;
	}

	if (memcmp(cdata->signature, CROSSYSTEM_DATA_SIGNATURE,
				sizeof(CROSSYSTEM_DATA_SIGNATURE))) {
		VBDEBUG(PREFIX "invalid signature: \"%s\"\n", cdata->signature);
		return 1;
	}

	if (cdata->version != CROSSYSTEM_DATA_VERSION) {
		VBDEBUG(PREFIX "version mismatch: %08x != %08x\n",
				cdata->version, CROSSYSTEM_DATA_VERSION);
		return 1;
	}

	/* Okay, the crossystem data blob passes the sanity check */
	return 0;
}

int crossystem_data_set_main_firmware(crossystem_data_t *cdata,
		uint8_t firmware_type,
		uint8_t *firmware_id)
{
	cdata->firmware_type = firmware_type;
	memcpy(cdata->firmware_id, firmware_id, sizeof(cdata->firmware_id));
	return 0;
}

#ifdef CONFIG_OF_LIBFDT
int crossystem_data_embed_into_fdt(crossystem_data_t *cdata, void *fdt,
		uint32_t *size_ptr)
{
	char path[] = "/firmware/chromeos";
	int nodeoffset, err;
	int gpio_phandle;
	int gpio_prop[3];

	err = fdt_open_into(fdt, fdt,
			fdt_totalsize(fdt) + sizeof(*cdata) + 4096);
	if (err < 0) {
		VBDEBUG(PREFIX "fail to resize fdt: %s\n", fdt_strerror(err));
		return 1;
	}
	*size_ptr = fdt_totalsize(fdt);

	/* TODO: Upstream device tree is moving from tegra250 to
	 * tegra20. Keep the check for 250 around for now but it can be
	 * removed once the changes have trickled down.
	 */
	nodeoffset = fdt_node_offset_by_compatible(fdt, 0,
						   "nvidia,tegra20-gpio");
	if (nodeoffset <= 0)
		nodeoffset = fdt_node_offset_by_compatible(fdt, 0,
						   "nvidia,tegra250-gpio");

	gpio_phandle = fdt_get_phandle(fdt, nodeoffset);
	if (gpio_phandle <= 0) {
		gpio_phandle = fdt_alloc_phandle(fdt);
		fdt_setprop_cell(fdt, nodeoffset,
				 "linux,phandle", gpio_phandle);
	}
	gpio_prop[0] = cpu_to_fdt32(gpio_phandle);

	nodeoffset = fdt_add_subnodes_from_path(fdt, 0, path);
	if (nodeoffset < 0) {
		VBDEBUG(PREFIX "fail to create subnode %s: %s\n", path,
				fdt_strerror(nodeoffset));
		return 1;
	}

	fdt_setprop_string(fdt, nodeoffset, "compatible", "chromeos-firmware");

#define set_scalar_prop(name, f) \
	fdt_setprop_cell(fdt, nodeoffset, name, cdata->f)
#define set_array_prop(name, f) \
	fdt_setprop(fdt, nodeoffset, name, cdata->f, sizeof(cdata->f))
#define set_conststring_prop(name, str) \
	fdt_setprop_string(fdt, nodeoffset, name, str)
#define set_bool_prop(name, f) \
	((cdata->f) ? fdt_setprop(fdt, nodeoffset, name, NULL, 0) : 0)

	err = 0;
	err |= set_scalar_prop("total-size", total_size);
	err |= set_array_prop("signature", signature);
	err |= set_scalar_prop("version", version);

	err |= set_bool_prop("boot-write-protect-switch",
			boot_write_protect_switch);
	err |= set_bool_prop("boot-recovery-switch",
			boot_recovery_switch);
	err |= set_bool_prop("boot-developer-switch",
			boot_developer_switch);

	gpio_prop[1] = cpu_to_fdt32(cdata->gpio_port_write_protect_switch);
	gpio_prop[2] = cpu_to_fdt32(cdata->polarity_write_protect_switch);
	err |= fdt_setprop(fdt, nodeoffset, "write-protect-switch",
			   gpio_prop, sizeof(gpio_prop));

	gpio_prop[1] = cpu_to_fdt32(cdata->gpio_port_recovery_switch);
	gpio_prop[2] = cpu_to_fdt32(cdata->polarity_recovery_switch);
	err |= fdt_setprop(fdt, nodeoffset, "recovery-switch",
			   gpio_prop, sizeof(gpio_prop));

	gpio_prop[1] = cpu_to_fdt32(cdata->gpio_port_developer_switch);
	gpio_prop[2] = cpu_to_fdt32(cdata->polarity_developer_switch);
	err |= fdt_setprop(fdt, nodeoffset, "developer-switch",
			   gpio_prop, sizeof(gpio_prop));

	err |= set_scalar_prop("fmap-offset", fmap_offset);

	switch (cdata->active_ec_firmware) {
	case ACTIVE_EC_FIRMWARE_RO:
		err |= set_conststring_prop("active-ec-firmware", "RO");
		break;
	case ACTIVE_EC_FIRMWARE_RW:
		err |= set_conststring_prop("active-ec-firmware", "RW");
		break;
	}

	switch (cdata->firmware_type) {
	case FIRMWARE_TYPE_RECOVERY:
		err |= set_conststring_prop("firmware-type", "recovery");
		break;
	case FIRMWARE_TYPE_NORMAL:
		err |= set_conststring_prop("firmware-type", "normal");
		break;
	case FIRMWARE_TYPE_DEVELOPER:
		err |= set_conststring_prop("firmware-type", "developer");
		break;
	}

	err |= set_array_prop("hardware-id", hardware_id);
	err |= set_array_prop("firmware-version", firmware_id);
	err |= set_array_prop("readonly-firmware-version",
			readonly_firmware_id);

#ifdef CONFIG_TEGRA2
	err |= set_scalar_prop("nonvolatile-context-lba",
			board.arm.nonvolatile_context_lba);
	err |= set_scalar_prop("nonvolatile-context-offset",
			board.arm.nonvolatile_context_offset);
	err |= set_scalar_prop("nonvolatile-context-size",
			board.arm.nonvolatile_context_size);
#endif

	err |= set_array_prop("vboot-shared-data", vb_shared_data);

#undef set_scalar_prop
#undef set_array_prop
#undef set_conststring_prop
#undef set_bool_prop

	if (err)
		VBDEBUG(PREFIX "fail to store all properties into fdt\n");
	return err;
}
#endif /* ^^^^ CONFIG_OF_LIBFDT  NOT defined ^^^^ */

#ifdef CONFIG_X86

static int crossystem_fw_index_vdat_to_binf(int index)
{
	switch (index) {
	case VDAT_RW_A:     return BINF_RW_A;
	case VDAT_RW_B:     return BINF_RW_B;
	case VDAT_RECOVERY: return BINF_RECOVERY;
	default:            return BINF_RECOVERY;
	}
};

int crossystem_data_update_acpi(crossystem_data_t *cdata)
{
	const void *fdt = gd->blob;
	int node_offset, len;
	const uint32_t *cell;
	chromeos_acpi_t *acpi_table;
	VbSharedDataHeader *vdat = (VbSharedDataHeader *)lib_sysinfo.vdat_addr;

	node_offset = fdt_path_offset(fdt, "/chromeos-config");
	if (node_offset < 0) {
		VBDEBUG("crossystem_data_update_acpi: Couldn't access "
			"chromeos-config.\n");
		return 1;
	}
	cell = fdt_getprop(fdt, node_offset, "gnvs-vboot-table", NULL);
	if (!cell) {
		VBDEBUG("crossystem_data_update_acpi: Couldn't access "
			"gnvs-vboot-table.\n");
		return 1;
	}
	acpi_table = (chromeos_acpi_t *)(uintptr_t)ntohl(*cell);

	acpi_table->vbt0 = BOOT_REASON_OTHER;
	acpi_table->vbt1 =
		crossystem_fw_index_vdat_to_binf(vdat->firmware_index);
	/* active_ec_firmware(vbt2) is set up by coreboot, so we don't
	 * set it up here on purpose. */
	acpi_table->vbt3 =
		(cdata->boot_write_protect_switch ? CHSW_FIRMWARE_WP_DIS : 0) |
		(cdata->boot_recovery_switch ? CHSW_RECOVERY_X86 : 0) |
		(cdata->boot_developer_switch ? CHSW_DEVELOPER_SWITCH : 0);

	len = min(ID_LEN, sizeof(acpi_table->vbt4));
	memcpy(acpi_table->vbt4, cdata->hardware_id, len);
	len = min(ID_LEN, sizeof(acpi_table->vbt5));
	memcpy(acpi_table->vbt5, cdata->firmware_id, len);
	len = min(ID_LEN, sizeof(acpi_table->vbt6));
	memcpy(acpi_table->vbt6, cdata->readonly_firmware_id, len);

	acpi_table->vbt7 = cdata->firmware_type;
	acpi_table->vbt8 = RECOVERY_REASON_NONE;
	acpi_table->vbt9 = cdata->fmap_offset;

	strncpy((char *)acpi_table->vbt10,
			(const char *)cdata->firmware_id, 64);
	return 0;
}
#endif

void crossystem_data_dump(crossystem_data_t *cdata)
{
#define _p(format, field) \
	VBDEBUG("crossystem_data_dump: %-30s: " format "\n", #field, cdata->field)
	_p("%08x",	total_size);
	_p("\"%s\"",	signature);
	_p("%d",	version);

	_p("%d",	boot_write_protect_switch);
	_p("%d",	boot_recovery_switch);
	_p("%d",	boot_developer_switch);
	_p("%d",	polarity_write_protect_switch);
	_p("%d",	polarity_recovery_switch);
	_p("%d",	polarity_developer_switch);
	_p("%d",	gpio_port_write_protect_switch);
	_p("%d",	gpio_port_recovery_switch);
	_p("%d",	gpio_port_developer_switch);

	_p("%08x",	fmap_offset);

	_p("%d",	active_ec_firmware);
	_p("%d",	firmware_type);
	_p("\"%s\"",	hardware_id);
	_p("\"%s\"",	readonly_firmware_id);
	_p("\"%s\"",	firmware_id);

#ifdef CONFIG_TEGRA2
	_p("%08llx",	board.arm.nonvolatile_context_lba);
	_p("%08x",	board.arm.nonvolatile_context_offset);
	_p("%08x",	board.arm.nonvolatile_context_size);
#endif
#undef _p
}
