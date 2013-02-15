/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef CHROMEOS_CROS_SPLASH_H_
#define CHROMEOS_CROS_SPLASH_H_

/**
 * Display splash image on screen.
 *
 * @param index		splash image index
 * @return 0 on success, non-0 on error
 */
int cros_splash_display(unsigned int index);

#endif /* CHROMEOS_CROS_SPLASH_H_ */
