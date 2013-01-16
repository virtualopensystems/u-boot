/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef __MKBP_TPS65090_H_
#define __MKBP_TPS65090_H_

/**
 * Enable FET
 *
 * @param fet_id	FET ID, value between 1 and 7
 * @return		0 on success, non-0 on failure
 */
int mkbp_tps65090_fet_enable(unsigned int fet_id);

/**
 * Disable FET
 *
 * @param fet_id	FET ID, value between 1 and 7
 * @return		0 on success, non-0 on failure
 */
int mkbp_tps65090_fet_disable(unsigned int fet_id);

/**
 * Is FET enabled?
 *
 * @param fet_id	FET ID, value between 1 and 7
 * @return		1 enabled, 0 disabled, negative value on failure
 */
int mkbp_tps65090_fet_is_enabled(unsigned int fet_id);

/**
 * Initialize the TPS65090 PMU using MKBP communication protocol.
 *
 * @return	0 on success, non-0 on failure
 */
int mkbp_tps65090_init(void);

#endif /* __MKBP_TPS65090_H_ */
