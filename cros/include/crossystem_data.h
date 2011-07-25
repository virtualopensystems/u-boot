/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef __CHROMEOS_CROSSYSTEM_DATA_H__
#define __CHROMEOS_CROSSYSTEM_DATA_H__

#include <chromeos/cros_gpio.h>
#include <vboot_nvstorage.h>
#include <vboot_struct.h>

#define ID_LEN		256

enum {
	ACTIVE_EC_FIRMWARE_RO = 0,
	ACTIVE_EC_FIRMWARE_RW = 1
};

enum {
	FIRMWARE_TYPE_RECOVERY	= 0,
	FIRMWARE_TYPE_NORMAL	= 1,
	FIRMWARE_TYPE_DEVELOPER	= 2
};

/* the data blob format */
typedef struct {
	/* Header of crossystem data blob */
	uint32_t	total_size;
	uint8_t		signature[10];
	uint16_t	version;

	/*
	 * Chrome OS-required GPIOs:
	 * - boot_*      : GPIO truth value at boot time
	 * - polarity_*  : Polarity (1=high active) of the GPIO pin
	 * - gpio_port_* : Port of the GPIO pin
	 */
	uint8_t		boot_write_protect_switch;
	uint8_t		boot_recovery_switch;
	uint8_t		boot_developer_switch;
	uint8_t		pad1[1];
	uint8_t		polarity_write_protect_switch;
	uint8_t		polarity_recovery_switch;
	uint8_t		polarity_developer_switch;
	uint8_t		pad2[1];
	uint32_t	gpio_port_write_protect_switch;
	uint32_t	gpio_port_recovery_switch;
	uint32_t	gpio_port_developer_switch;
	uint8_t		pad3[4];

	/* Offset of FMAP on flashrom */
	uint32_t	fmap_offset;

	/*
	 * Firmware and system information:
	 * - active_ec_firmware   : 0=RO, 1=RW
	 * - firmware_type        : 0=recovery, 1=normal, 2=developer
	 * - hardware_id          : The hardware ID of this machine
	 * - readonly_firmware_id : ID of the read-only firmware
	 * - firmware_id          : ID of the active main firmware
	 */
	uint8_t		active_ec_firmware;
	uint8_t		firmware_type;
	uint8_t		pad4[2];
	uint8_t 	hardware_id[ID_LEN];
	uint8_t		readonly_firmware_id[ID_LEN];
	uint8_t		firmware_id[ID_LEN];

	union {
		/* We reserve 208 bytes for board specific data */
		uint8_t board_reserved_size[0xd0];

		struct {
			/* Location of non-volatile context */
			uint64_t	nonvolatile_context_lba;
			uint16_t	nonvolatile_context_offset;
			uint16_t	nonvolatile_context_size;
		} arm;
	} board;

	/*
	 * VbSharedData contains fields of long word (64-bit) type, and so it
	 * should better be aligned on a 8-byte boundary on ARM platforms.
	 * Although ARM processors can access unaligned addresses, this feature
	 * is generally not enabled in U-Boot.
	 */
	uint8_t		vb_shared_data[VB_SHARED_DATA_REC_SIZE];
} __attribute__((packed)) crossystem_data_t;

#define assert_offset(MEMBER, OFFSET) \
	typedef char static_assertion_##MEMBER_is_at_offset_##OFFSET[ \
		(offsetof(crossystem_data_t, MEMBER) == (OFFSET)) ? 1 : -1]

assert_offset(total_size,			0x0000);
assert_offset(signature,			0x0004);
assert_offset(version,				0x000e);

assert_offset(boot_write_protect_switch,	0x0010);
assert_offset(boot_recovery_switch,		0x0011);
assert_offset(boot_developer_switch,		0x0012);
assert_offset(polarity_write_protect_switch,	0x0014);
assert_offset(polarity_recovery_switch,		0x0015);
assert_offset(polarity_developer_switch,	0x0016);
assert_offset(gpio_port_write_protect_switch,	0x0018);
assert_offset(gpio_port_recovery_switch,	0x001c);
assert_offset(gpio_port_developer_switch,	0x0020);

assert_offset(fmap_offset,			0x0028);

assert_offset(active_ec_firmware,		0x002c);
assert_offset(firmware_type,			0x002d);
assert_offset(hardware_id,			0x0030);
assert_offset(readonly_firmware_id,		0x0130);
assert_offset(firmware_id,			0x0230);

assert_offset(board.arm.nonvolatile_context_lba,	0x0330);
assert_offset(board.arm.nonvolatile_context_offset,	0x0338);
assert_offset(board.arm.nonvolatile_context_size,	0x033a);

assert_offset(vb_shared_data,			0x0400);

#undef assert_offset

/**
 * This initializes the data blob that we will pass to kernel, and later be
 * used by crossystem. Note that:
 * - It does not initialize information of the main firmware, e.g., fwid. This
 *   information must be initialized in subsequent calls to the setters below.
 * - The recovery reason is default to VBNV_RECOVERY_NOT_REQUESTED.
 *
 * @param cdata is the data blob shared with crossystem
 * @param write_protect_switch points to a GPIO descriptor
 * @param recovery_switch points to a GPIO descriptor
 * @param developer_switch points to a GPIO descriptor
 * @param fmap_offset is the offset of FMAP in flashrom
 * @param hardware_id is of length ID_LEN
 * @param readonly_firmware_id is of length ID_LEN
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_init(crossystem_data_t *cdata,
		cros_gpio_t *write_protect_switch,
		cros_gpio_t *recovery_switch,
		cros_gpio_t *developer_switch,
		uint32_t fmap_offset,
		uint8_t active_ec_firmware,
		uint8_t *hardware_id,
		uint8_t *readonly_firmware_id);

/**
 * This checks sanity of the crossystem data blob. Readwrite main firmware
 * should check the sanity of crossystem data that bootstub passes to it.
 *
 * @param cdata is the data blob shared with crossystem
 * @return 0 if it succeeds; non-zero if the sanity check fails
 */
int crossystem_data_check_integrity(crossystem_data_t *cdata);

/**
 * This sets the main firmware version and type.
 *
 * @param cdata is the data blob shared with crossystem
 * @param firmware_type
 * @param firmware_id is of length ID_LEN
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_set_main_firmware(crossystem_data_t *cdata,
		uint8_t firmware_type,
		uint8_t *firmware_id);

/**
 * This embeds kernel shared data into fdt.
 *
 * @param cdata is the data blob shared with crossystem
 * @param fdt points to a device tree
 * @param size_ptr stores the new size of fdt
 * @return 0 if it succeeds, non-zero if it fails
 */
int crossystem_data_embed_into_fdt(crossystem_data_t *cdata, void *fdt,
		uint32_t *size_ptr);

/**
 * This prints out the data blob content to debug output.
 */
void crossystem_data_dump(crossystem_data_t *cdata);

#endif /* __CHROMEOS_CROSSYSTEM_DATA_H__ */
