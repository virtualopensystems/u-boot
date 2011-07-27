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
#include <chromeos/boot_kernel.h>
#include <chromeos/common.h>
#include <tss_constants.h>
#include <vboot/entry_points.h>
#include <vboot/global_data.h>
#include <vboot_api.h>

#define PREFIX		"main: "

static void prepare_cparams(vb_global_t *global, VbCommonParams *cparams)
{
	cparams->gbb_data = global->gbb_data;
	cparams->gbb_size = global->gbb_size;
	cparams->shared_data_blob = global->cdata_blob.vb_shared_data;
	cparams->shared_data_size = VB_SHARED_DATA_REC_SIZE;
}

static void prepare_kparams(VbSelectAndLoadKernelParams *kparams)
{
	kparams->kernel_buffer = (void *)CONFIG_CHROMEOS_KERNEL_LOADADDR;
	kparams->kernel_buffer_size = CONFIG_CHROMEOS_KERNEL_BUFSIZE;
}

static VbError_t call_VbSelectAndLoadKernel(VbCommonParams* cparams,
					VbSelectAndLoadKernelParams* kparams)
{
        VbError_t ret;

        VBDEBUG("VbCommonParams:\n");
        VBDEBUG("    gbb_data         : %p\n", cparams->gbb_data);
        VBDEBUG("    gbb_size         : %u\n", cparams->gbb_size);
        VBDEBUG("    shared_data_blob : %p\n", cparams->shared_data_blob);
        VBDEBUG("    shared_data_size : %u\n", cparams->shared_data_size);
        VBDEBUG("    caller_context   : %p\n", cparams->caller_context);
        VBDEBUG("VbSelectAndLoadKernelParams:\n");
        VBDEBUG("    kernel_buffer      : %p\n", kparams->kernel_buffer);
        VBDEBUG("    kernel_buffer_size : %u\n",
						kparams->kernel_buffer_size);
        VBDEBUG("Calling VbSelectAndLoadKernel()...\n");

        ret = VbSelectAndLoadKernel(cparams, kparams);
        VBDEBUG("Returned %#x\n", ret);

	if (!ret) {
		int i;
	        VBDEBUG("VbSelectAndLoadKernelParams:\n");
		VBDEBUG("    disk_handle        : %p\n",
						kparams->disk_handle);
		VBDEBUG("    partition_number   : %u\n",
						kparams->partition_number);
		VBDEBUG("    bootloader_address : %#llx\n",
						kparams->bootloader_address);
		VBDEBUG("    bootloader_size    : %u\n",
						kparams->bootloader_size);
		VBDEBUG("    partition_guid     :");
		for (i = 0; i < 16; i++)
			VBDEBUG(" %02x", kparams->partition_guid[i]);
		VBDEBUG("\n");
	}

	return ret;
}

void main_entry(void)
{
	vb_global_t *global;
	VbCommonParams cparams;
	VbSelectAndLoadKernelParams kparams;
	VbError_t ret;

	/* Get VBoot Global Data which was initialized by bootstub. */
	global = get_vboot_global();
	if (!is_vboot_global_valid(global))
		VbExError(PREFIX "VBoot Global Data is not initialized!\n");

	prepare_cparams(global, &cparams);
	prepare_kparams(&kparams);

	/*
	 * VbSelectAndLoadKernel() assumes the TPM interface has already been
	 * initialized by VbSelectFirmware(). Since we haven't called
	 * VbSelectFirmware() in the readwrite firmware, we need to explicitly
	 * initialize the TPM interface. Note that this only re-initializes the
	 * interface, not the TPM itself.
	 */
	if (VbExTpmInit() != TPM_SUCCESS) {
		VbExError(PREFIX "failed to init tpm interface\n");
	}

	ret = call_VbSelectAndLoadKernel(&cparams, &kparams);

	if (ret)
		VbExError(PREFIX "Failed to select and load kernel!\n");

	/* Do boot partition substitution in kernel cmdline and boot */
	ret = boot_kernel(&kparams, &global->cdata_blob);

	VbExError(PREFIX "boot_kernel_helper returned, %u\n", ret);
}
