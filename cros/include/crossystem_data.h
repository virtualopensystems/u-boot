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

#define RECOVERY_FIRMWARE	0
#define REWRITABLE_FIRMWARE_A	1
#define REWRITABLE_FIRMWARE_B	2
#define READONLY_FIRMWARE	3

#define RECOVERY_TYPE		0
#define NORMAL_TYPE		1
#define DEVELOPER_TYPE		2

/* the data blob format */
typedef struct {
	uint32_t	total_size;				/* 0000 */
	uint8_t		signature[10];				/* 0004 */
	uint16_t	version;				/* 000c */

	uint64_t	nvcxt_lba;				/* 0010 */
	uint16_t	vbnv[2];				/* 0018 */
	uint8_t		nvcxt_cache[VBNV_BLOCK_SIZE];		/* 001c */

	uint8_t		write_protect_sw;			/* 002c */
	uint8_t		recovery_sw;				/* 002d */
	uint8_t		developer_sw;				/* 002e */
	uint8_t		pad1[1];

	uint32_t	gpio_port_write_protect_sw;		/* 0030 */
	uint32_t	gpio_port_recovery_sw;			/* 0034 */
	uint32_t	gpio_port_developer_sw;			/* 0038 */

	uint8_t		polarity_write_protect_sw;		/* 003c */
	uint8_t		polarity_recovery_sw;			/* 003d */
	uint8_t		polarity_developer_sw;			/* 003e */
	uint8_t		pad2[1];

	uint8_t		binf[5];				/* 0040 */
	uint8_t		pad3[1];
	uint32_t	chsw;					/* 0046 */

	uint8_t 	hwid[ID_LEN];				/* 004a */
	uint8_t		fwid[ID_LEN];				/* 014a */
	uint8_t		frid[ID_LEN];				/* 024a */

	uint32_t	fmap_base;				/* 034a */
	uint8_t		pad4[2];

	/*
	 * VbSharedData contains fields of long word (64-bit) type, and so it
	 * should better be aligned on a 8-byte boundary on ARM platforms.
	 * Although ARM processors can access unaligned addresses, this feature
	 * is generally not enabled in U-Boot.
	 */
	uint8_t		vbshared_data[VB_SHARED_DATA_REC_SIZE];	/* 0350 */
} __attribute__((packed)) crossystem_data_t;

/**
 * This initializes the data blob that we will pass to kernel, and later be
 * used by crossystem. Note that:
 * - It does not initialize information of the main firmware, e.g., fwid. This
 *   information must be initialized in subsequent calls to the setters below.
 * - The recovery reason is default to VBNV_RECOVERY_NOT_REQUESTED.
 *
 * @param cdata is the data blob shared with crossystem
 * @param frid r/o firmware id; a zero-terminated string shorter than ID_LEN
 * @param fmap_data is the address of fmap in firmware
 * @param gbb_data points to gbb blob
 * @param nvcxt_raw points to the VbNvContext raw data
 * @param wpsw stores the value of write protect gpio
 * @param recsw stores the value of recovery mode gpio
 * @param devsw stores the value of developer mode gpio
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_init(crossystem_data_t *cdata, uint8_t *frid,
		uint32_t fmap_data, void *gbb_data, void *nvcxt_raw,
		cros_gpio_t *wpsw, cros_gpio_t *recsw, cros_gpio_t *devsw);

/**
 * This checks sanity of the crossystem data blob. Readwrite main firmware
 * should check the sanity of crossystem data that bootstub passes to it.
 *
 * @param cdata is the data blob shared with crossystem
 * @return 0 if it succeeds; non-zero if the sanity check fails
 */
int crossystem_data_check_integrity(crossystem_data_t *cdata);

/**
 * This sets rewritable firmware id. It should only be called in non-recovery
 * boots.
 *
 * @param cdata is the data blob shared with crossystem
 * @param fwid r/w firmware id; a zero-terminated string shorter than ID_LEN
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_set_fwid(crossystem_data_t *cdata, uint8_t *fwid);

/**
 * This sets active main firmware and its type.
 *
 * @param cdata is the data blob shared with crossystem
 * @param which - recovery: 0; rewritable A: 1; rewritable B: 2
 * @param type - recovery: 0; normal: 1; developer: 2
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_set_active_main_firmware(crossystem_data_t *cdata,
		int which, int type);

int crossystem_data_get_active_main_firmware(crossystem_data_t *cdata);
int crossystem_data_get_active_main_firmware_type(crossystem_data_t *cdata);

/**
 * This sets recovery reason.
 *
 * @param cdata is the data blob shared with crossystem
 * @param reason - the recovery reason
 * @return 0 if it succeeds; non-zero if it fails
 */
int crossystem_data_set_recovery_reason(crossystem_data_t *cdata,
		uint32_t reason);

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
