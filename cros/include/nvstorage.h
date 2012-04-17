/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CHROMEOS_NVSTORAGE_H_
#define CHROMEOS_NVSTORAGE_H_

/*
 * The VbNvContext is stored in block 0, which is also the MBR on x86
 * platforms but generally unused on ARM platforms.  Given this, it is not a
 * perfect place for storing stuff, but since there are no fixed blocks that we
 * may use reliably, block 0 is our only option left.
 */
#define CHROMEOS_VBNVCONTEXT_LBA	0

#endif /* CHROMEOS_NVSTORAGE_H_ */
