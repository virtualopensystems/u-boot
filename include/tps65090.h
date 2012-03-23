/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ifndef __TPS65090_H_
#define __TPS65090_H_

/* I2C device address for TPS65090 PMU */
#define TPS65090_I2C_ADDR	0x48

/* TPS65090 register addresses */
#define TPS65090_REG_FET1_CTRL	0x0f
#define TPS65090_REG_FET2_CTRL	0x10
#define TPS65090_REG_FET3_CTRL	0x11
#define TPS65090_REG_FET4_CTRL	0x12
#define TPS65090_REG_FET5_CTRL	0x13
#define TPS65090_REG_FET6_CTRL	0x14
#define TPS65090_REG_FET7_CTRL	0x15

#define TPS65090_MAX_FET_NUM	7

/* TPS65090 FET_CTRL register values */
#define TPS65090_FET_CTRL_PGFET		0x10  /* Power good for FET status */
#define TPS65090_FET_CTRL_ADENFET	0x02  /* Enable output auto discharge */
#define TPS65090_FET_CTRL_ENFET		0x01  /* Enable FET */

/**
 * Enable FET
 *
 * @param	fet_id	FET ID, value between 1 and 7
 * @return	0 on success, non-0 on failure
 */
int tps65090_fet_enable(unsigned int fet_id);

/**
 * Disable FET
 *
 * @param	fet_id	FET ID, value between 1 and 7
 * @return	0 on success, non-0 on failure
 */
int tps65090_fet_disable(unsigned int fet_id);

/**
 * Is FET enabled?
 *
 * @param	fet_id	FET ID, value between 1 and 7
 * @return	1 enabled, 0 disabled, negative value on failure
 */
int tps65090_fet_is_enabled(unsigned int fet_id);

/**
 * Initialize the TPS65090 PMU.
 *
 * @return	0 on success, non-0 on failure
 */
int tps65090_init(void);

#endif /* __TPS65090_H_ */
