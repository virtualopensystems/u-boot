/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* GPIO interface for Chrome OS verified boot */

#ifndef __VBOOT_FLAG_H__
#define __VBOOT_FLAG_H__

enum vboot_flag_id {
	VBOOT_FLAG_WRITE_PROTECT = 0,
	VBOOT_FLAG_RECOVERY,
	VBOOT_FLAG_DEVELOPER,
	VBOOT_FLAG_LID_OPEN,
	VBOOT_FLAG_POWER_OFF,

	VBOOT_FLAG_MAX_FLAGS
};

enum cros_gpio_polarity {
	CROS_GPIO_ACTIVE_LOW	= 0,
	CROS_GPIO_ACTIVE_HIGH	= 1
};

typedef struct {
	enum vboot_flag_id id;
	int port;
	int polarity;
	int value;
} cros_gpio_t;

/**
 * Initialize cros_gpio module
 *
 * @return zero on success and non-zero on failures
 */
int cros_gpio_init(void);

/**
 * Board-specific setup of GPIO ports that is called from the generic
 * cros_gpio_init.
 *
 * @param id		ID of VBoot flag
 * @param port		GPIO port
 * @return zero on success and non-zero on failures
 */
int cros_gpio_setup(enum vboot_flag_id id, int port);

int cros_gpio_fetch(enum vboot_flag_id id, cros_gpio_t *gpio);

int cros_gpio_dump(cros_gpio_t *gpio);

#endif /* __VBOOT_FLAG_H__ */
