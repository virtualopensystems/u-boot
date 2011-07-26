/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CHROMEOS_GBB_H_
#define CHROMEOS_GBB_H_

#include <chromeos/firmware_storage.h>

/**
 * This loads the basic parts of GBB from flashrom. These include:
 *  - GBB header
 *  - hardware id
 *  - rootkey for verifying readwrite firmware
 *
 * @param gbb		Buffer for holding GBB
 * @param file		Flashrom device handle
 * @param gbb_offset	Offset of GBB in flashrom device
 * @return zero if this succeeds, non-zero if this fails
 */
int gbb_init(void *gbb, firmware_storage_t *file, uint32_t gbb_offset);

/**
 * This loads the BMP block of GBB from flashrom.
 *
 * @param gbb		Buffer for holding GBB
 * @param file		Flashrom device handle
 * @param gbb_offset	Offset of GBB in flashrom device
 * @return zero if this succeeds, non-zero if this fails
 */
int gbb_read_bmp_block(void *gbb,
		firmware_storage_t *file, uint32_t gbb_offset);

/*
 * This loads the recovery key of GBB from flashrom.
 *
 * @param gbb		Buffer for holding GBB
 * @param file		Flashrom device handle
 * @param gbb_offset	Offset of GBB in flashrom device
 * @return zero if this succeeds, non-zero if this fails
 */
int gbb_read_recovery_key(void *gbb,
		firmware_storage_t *file, uint32_t gbb_offset);

#endif /* CHROMEOS_GBB_H_ */
