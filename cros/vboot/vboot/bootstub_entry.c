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
#include <chromeos/common.h>
#include <fdt_decode.h>
#include <chromeos/crossystem_data.h>
#include <chromeos/fdt_decode.h>
#include <chromeos/firmware_storage.h>
#include <chromeos/gbb.h>
#include <chromeos/power_management.h>
#include <chromeos/memory_wipe.h>
#include <vboot/entry_points.h>
#include <vboot/firmware_cache.h>
#include <vboot/global_data.h>

#include <vboot_api.h>

#define PREFIX			"bootstub: "

DECLARE_GLOBAL_DATA_PTR;

/* The margin to keep extra stack region that not to be wiped. */
#define STACK_MARGIN		1024

static void prepare_cparams(vb_global_t *global, VbCommonParams *cparams)
{
	cparams->gbb_data = global->gbb_data;
	cparams->gbb_size = global->gbb_size;
	cparams->shared_data_blob = global->cdata_blob.vb_shared_data;
	cparams->shared_data_size = VB_SHARED_DATA_REC_SIZE;
}

static void prepare_iparams(vb_global_t *global, VbInitParams *iparams)
{
	crossystem_data_t *cdata = &global->cdata_blob;
	iparams->flags = VB_INIT_FLAG_RO_NORMAL_SUPPORT;
	if (cdata->boot_developer_switch)
		iparams->flags |= VB_INIT_FLAG_DEV_SWITCH_ON;
	if (cdata->boot_recovery_switch)
		iparams->flags |= VB_INIT_FLAG_REC_BUTTON_PRESSED;
	if (cdata->boot_write_protect_switch)
		iparams->flags |= VB_INIT_FLAG_WP_ENABLED;
}

static int read_verification_block(firmware_storage_t *file,
		const off_t vblock_offset, void **vblock_ptr,
		uint32_t *vblock_size_ptr, uint32_t *body_size_ptr)
{
	VbKeyBlockHeader kbh;
	VbFirmwarePreambleHeader fph;
	uint32_t key_block_size, vblock_size;
	void *vblock;

	/* read key block header */
	if (file->read(file, vblock_offset, sizeof(kbh), &kbh)) {
		VBDEBUG(PREFIX "Failed to read key block!\n");
		return -1;
	}
	key_block_size = kbh.key_block_size;

	/* read firmware preamble header */
	if (file->read(file, vblock_offset + key_block_size,
				sizeof(fph), &fph)) {
		VBDEBUG(PREFIX "Failed to read preamble!\n");
		return -1;
	}
	vblock_size = key_block_size + fph.preamble_size;

	vblock = VbExMalloc(vblock_size);

	if (file->read(file, vblock_offset, vblock_size, vblock)) {
		VBDEBUG(PREFIX "Failed to read verification block!\n");
		VbExFree(vblock);
		return -1;
	}

	*vblock_ptr = vblock;
	*vblock_size_ptr = vblock_size;
	*body_size_ptr = fph.body_signature.data_size;

	return 0;
}

static void prepare_fparams(firmware_storage_t *file,
			    firmware_cache_t *cache,
			    struct twostop_fmap *fmap,
			    VbSelectFirmwareParams *fparams)
{
	uint32_t fw_main_a_size, fw_main_b_size;

	if (read_verification_block(file,
			fmap->readwrite_a.vblock.offset,
			&fparams->verification_block_A,
			&fparams->verification_size_A,
			&fw_main_a_size))
		VbExError(PREFIX "Failed to read verification block A!\n");

	if (read_verification_block(file,
			fmap->readwrite_b.vblock.offset,
			&fparams->verification_block_B,
			&fparams->verification_size_B,
			&fw_main_b_size))
		VbExError(PREFIX "Failed to read verification block B!\n");

	/* Prepare the firmware cache which is passed as caller_context. */
	init_firmware_cache(cache,
			file,
			fmap->readwrite_a.boot.offset,
			fw_main_a_size,
			fmap->readwrite_b.boot.offset,
			fw_main_b_size);
}

static void release_fparams(VbSelectFirmwareParams *fparams)
{
	VbExFree(fparams->verification_block_A);
	VbExFree(fparams->verification_block_B);
}

#ifdef CONFIG_OF_CONTROL
static uintptr_t get_current_sp(void)
{
	uintptr_t addr;

	addr = (uintptr_t)&addr;
	return addr;
}
#endif

static void wipe_unused_memory(const void const *fdt_ptr, vb_global_t *global)
{
#ifdef CONFIG_OF_CONTROL
	memory_wipe_t wipe;
	struct fdt_memory config;

	if (fdt_decode_memory(fdt_ptr, &config))
		VbExError(PREFIX "FDT decode memory section error\n");

	memory_wipe_init(&wipe, config.start, config.end);

	/* Excludes stack, fdt, gd, bd, heap, u-boot, framebuffer, etc. */
	memory_wipe_exclude(&wipe, get_current_sp() - STACK_MARGIN, config.end);

	/* Excludes the shared data between bootstub and main firmware. */
	memory_wipe_exclude(&wipe, (uintptr_t)global,
			(uintptr_t)global + sizeof(*global));

	/* Excludes the LP0 vector. */
	memory_wipe_exclude(&wipe,
			    (uintptr_t)TEGRA_LP0_ADDR,
			    (uintptr_t)(TEGRA_LP0_ADDR + TEGRA_LP0_SIZE));

	memory_wipe_execute(&wipe);
#else
	printf("wipe_unused_memory depends on fdt_decode_memory which"
		" isn't configured\n");
#endif
}

typedef void (*firmware_entry_t)(void);

static void jump_to_firmware(firmware_entry_t firmware_entry)
{
	VBDEBUG(PREFIX "Jump to firmware %p...\n", firmware_entry);

	cleanup_before_linux();

	/* Jump and never return */
	(*firmware_entry)();

	VbExError(PREFIX "Firmware %p returned!\n", firmware_entry);
}

static VbError_t call_VbInit(VbCommonParams *cparams, VbInitParams *iparams)
{
	VbError_t ret;

	VBDEBUG("VbCommonParams:\n");
	VBDEBUG("    gbb_data         : %p\n", cparams->gbb_data);
	VBDEBUG("    gbb_size         : %u\n", cparams->gbb_size);
	VBDEBUG("    shared_data_blob : %p\n", cparams->shared_data_blob);
	VBDEBUG("    shared_data_size : %u\n", cparams->shared_data_size);
	VBDEBUG("    caller_context   : %p\n", cparams->caller_context);
	VBDEBUG("VbInitParams:\n");
	VBDEBUG("    flags         : %#x\n", iparams->flags);
	VBDEBUG("Calling VbInit()...\n");

	ret = VbInit(cparams, iparams);
	VBDEBUG("Returned %#x\n", ret);

	if (!ret) {
		VBDEBUG("VbInitParams:\n");
		VBDEBUG("    out_flags     : %#x\n", iparams->out_flags);
	}

	return ret;
}

static VbError_t call_VbSelectFirmware(VbCommonParams *cparams,
                                       VbSelectFirmwareParams *fparams)
{
	VbError_t ret;

	VBDEBUG("VbCommonParams:\n");
	VBDEBUG("    gbb_data         : %p\n", cparams->gbb_data);
	VBDEBUG("    gbb_size         : %u\n", cparams->gbb_size);
	VBDEBUG("    shared_data_blob : %p\n", cparams->shared_data_blob);
	VBDEBUG("    shared_data_size : %u\n", cparams->shared_data_size);
	VBDEBUG("    caller_context   : %p\n", cparams->caller_context);

	VBDEBUG("VbSelectFirmwareParams:\n");
	VBDEBUG("    verification_block_A : %p\n",
			fparams->verification_block_A);
	VBDEBUG("    verification_block_B : %p\n",
			fparams->verification_block_B);
	VBDEBUG("    verification_size_A  : %u\n",
			fparams->verification_size_A);
	VBDEBUG("    verification_size_B  : %u\n",
			fparams->verification_size_B);
	VBDEBUG("Calling VbSelectFirmware()...\n");

	ret = VbSelectFirmware(cparams, fparams);
	VBDEBUG("Returned %#x\n", ret);

	if (!ret) {
		VBDEBUG("VbSelectFirmwareParams:\n");
		VBDEBUG("    selected_firmware    : %u\n",
				fparams->selected_firmware);
	}

	return ret;
}

static int fill_crossystem_data(vb_global_t *global,
				firmware_storage_t *file,
				struct twostop_fmap *fmap,
				uint32_t selected_firmware)
{
	crossystem_data_t *cdata = &global->cdata_blob;
	uint8_t fwid_buf[ID_LEN];
	uint8_t *fwid = fwid_buf;
	uint32_t fwid_offset;
	/* mainfw_type of non-recovery boot depends on dev switch */
	int mainfw_type = cdata->boot_developer_switch ?
			FIRMWARE_TYPE_DEVELOPER : FIRMWARE_TYPE_NORMAL;

	/* Fills FWID */
	switch (selected_firmware) {
	case VB_SELECT_FIRMWARE_RECOVERY:
		mainfw_type = FIRMWARE_TYPE_RECOVERY;
		/* continue */

	case VB_SELECT_FIRMWARE_READONLY:
		fwid = cdata->readonly_firmware_id;
		goto done;

	case VB_SELECT_FIRMWARE_A:
		fwid_offset = fmap->readwrite_a.firmware_id.offset;
		assert(fmap->readwrite_a.firmware_id.length == ID_LEN);
		break;

	case VB_SELECT_FIRMWARE_B:
		fwid_offset = fmap->readwrite_b.firmware_id.offset;
		assert(fmap->readwrite_b.firmware_id.length == ID_LEN);
		break;

	default:
		return 1;
	}

	if (file->read(file, fwid_offset, ID_LEN, fwid_buf)) {
		VBDEBUG(PREFIX "Failed to read FWID from firmware!\n");
		return 1;
	}

done:
	crossystem_data_set_main_firmware(cdata, mainfw_type, fwid);
	crossystem_data_dump(cdata);

	return 0;
}

void bootstub_entry(void)
{
	void *fdt_ptr = (void *)gd->blob;
	struct twostop_fmap fmap;
	vb_global_t *global;
	firmware_storage_t file;
	firmware_cache_t cache;
	VbCommonParams cparams;
	VbInitParams iparams;
	VbSelectFirmwareParams fparams;
	VbError_t ret;

	if (fdt_decode_twostop_fmap(fdt_ptr, &fmap))
		VbExError(PREFIX "Failed to load fmap config from fdt.\n");

	/* Open firmware storage device */
	if (firmware_storage_open_spi(&file))
		VbExError(PREFIX "Failed to open firmware device!\n");

	/* Init VBoot Global Data and load GBB from SPI */
	global = get_vboot_global();
	if (init_vboot_global(global, &file))
		VbExError(PREFIX "Failed to init vboot global data!\n");

	/* Call VbInit() */
	prepare_cparams(global, &cparams);
	prepare_iparams(global, &iparams);
	if ((ret = call_VbInit(&cparams, &iparams)))
		VbExError(PREFIX "VbInit() returns error: %#x!\n", ret);

	/* Handle the VbInit() results */
	if (iparams.out_flags & VB_INIT_OUT_CLEAR_RAM)
		wipe_unused_memory(fdt_ptr, global);
	if (iparams.out_flags & VB_INIT_OUT_ENABLE_DISPLAY)
		if (gbb_read_bmp_block(global->gbb_data, &file,
					fmap.readonly.gbb.offset))
			VbExError(PREFIX "Failed to load BMP Block!\n");
	if (iparams.out_flags & VB_INIT_OUT_ENABLE_RECOVERY)
		if (gbb_read_recovery_key(global->gbb_data, &file,
					fmap.readonly.gbb.offset))
			VbExError(PREFIX "Failed to load recovery key!\n");

	/* Call VbSelectFirmware() */
	cparams.caller_context = &cache;
	prepare_fparams(&file, &cache, &fmap, &fparams);
	if ((ret = call_VbSelectFirmware(&cparams, &fparams)))
		VbExError(PREFIX "VbSelectFirmare() returned error: %#x!\n",
				ret);
	release_fparams(&fparams);

	if (fill_crossystem_data(global, &file, &fmap,
				 fparams.selected_firmware))
		VbExError(PREFIX "Failed to fill crossystem data!\n");

	if (file.close(&file))
		VbExError(PREFIX "Failed to close firmware device!\n");

	/* Handle the VbSelectFirmware() results */
	switch (fparams.selected_firmware) {
	case VB_SELECT_FIRMWARE_A:
		memcpy((void *)CONFIG_SYS_TEXT_BASE, cache.infos[0].buffer,
				cache.infos[0].size);
		jump_to_firmware((firmware_entry_t)CONFIG_SYS_TEXT_BASE);
		break;

	case VB_SELECT_FIRMWARE_B:
		memcpy((void *)CONFIG_SYS_TEXT_BASE, cache.infos[1].buffer,
				cache.infos[1].size);
		jump_to_firmware((firmware_entry_t)CONFIG_SYS_TEXT_BASE);
		break;

	case VB_SELECT_FIRMWARE_RECOVERY:
		VBDEBUG(PREFIX "Boot to recovery mode...\n");
		main_entry();
		break;

	case VB_SELECT_FIRMWARE_READONLY:
		VBDEBUG(PREFIX "Boot to RO firmware...\n");
		main_entry();
		break;

	default:
		VbExError(PREFIX "Unexpected selected firmware value!\n");
	}

	VbExError(PREFIX "Should not reach here!\n");
}
