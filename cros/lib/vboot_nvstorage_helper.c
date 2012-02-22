/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 * * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <cros/power_management.h>
#include <cros/vboot_nvstorage_helper.h>

/*
 * TODO It should averagely distributed erase/write operation to entire flash
 * memory section allocated for VBNVCONTEXT to increase maximal lifetime.
 *
 * But since VbNvContext gets written infrequently enough, this is likely
 * an overkill.
 */

#define PREFIX "vboot_nvstorage_helper: "

int read_nvcontext(firmware_storage_t *file, VbNvContext *nvcxt)
{
	if (firmware_storage_read(file,
			CONFIG_OFFSET_VBNVCONTEXT, VBNV_BLOCK_SIZE,
			nvcxt->raw)) {
		debug(PREFIX "read_nvcontext fail\n");
		return -1;
	}

	if (VbNvSetup(nvcxt)) {
		debug(PREFIX "setup nvcontext fail\n");
		return -1;
	}

	return 0;
}

int write_nvcontext(firmware_storage_t *file, VbNvContext *nvcxt)
{
	if (firmware_storage_write(file,
			CONFIG_OFFSET_VBNVCONTEXT, VBNV_BLOCK_SIZE,
			nvcxt->raw)) {
		debug(PREFIX "write_nvcontext fail\n");
		return -1;
	}

	return 0;
}

int clear_recovery_request(void)
{
	VbNvContext nvcxt;

	if (read_nvcontext(&nvcxt) || VbNvSetup(&nvcxt)) {
		debug(PREFIX "cannot read nvcxt\n");
		return 1;
	}

	if (VbNvSet(&nvcxt, VBNV_RECOVERY_REQUEST,
				VBNV_RECOVERY_NOT_REQUESTED)) {
		debug(PREFIX "cannot clear VBNV_RECOVERY_REQUEST\n");
		return 1;
	}

	if (VbNvTeardown(&nvcxt) ||
			(nvcxt.raw_changed && write_nvcontext(&nvcxt))) {
		debug(PREFIX "cannot write nvcxt\n");
		return 1;
	}

	return 0;
}

void reboot_to_recovery_mode(uint32_t reason)
{
	VbNvContext *nvcxt, nvcontext;

	nvcxt = &nvcontext;
	if (read_nvcontext(nvcxt) || VbNvSetup(nvcxt)) {
		debug(PREFIX "cannot read nvcxt\n");
		goto FAIL;
	}

	debug(PREFIX "store recovery cookie in recovery field\n");
	if (VbNvSet(nvcxt, VBNV_RECOVERY_REQUEST, reason) ||
			VbNvTeardown(nvcxt) ||
			(nvcxt->raw_changed && write_nvcontext(nvcxt))) {
		debug(PREFIX "cannot write back nvcxt");
		goto FAIL;
	}

	debug(PREFIX "reboot to recovery mode\n");
	cold_reboot();

	debug(PREFIX "error: cold_reboot() returned\n");
FAIL:
	/* FIXME: bring up a sad face? */
	printf("Please reset and press recovery button when reboot.\n");
	while (1);
}

char *nvcontext_to_str(VbNvContext *nvcxt)
{
	static char buf[VBNV_BLOCK_SIZE * 2];
	int i, j, x;

	for (i = 0; i < VBNV_BLOCK_SIZE; i++) {
		x = nvcxt->raw[i];
		j = i << 1;
		buf[j]   = "0123456789abcdef"[(x >> 4) & 0xf];
		buf[j+1] = "0123456789abcdef"[x & 0xf];
	}

	return buf;
}
