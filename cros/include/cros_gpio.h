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

#ifndef CROS_GPIO_H__
#define CROS_GPIO_H__

enum cros_gpio_index {
	CROS_GPIO_WPSW = 0,
	CROS_GPIO_RECSW,
	CROS_GPIO_DEVSW,
	CROS_GPIO_LIDSW,
	CROS_GPIO_PWRSW,

	CROS_GPIO_MAX_GPIO
};

enum cros_gpio_polarity {
	CROS_GPIO_ACTIVE_LOW	= 0,
	CROS_GPIO_ACTIVE_HIGH	= 1
};

typedef struct {
	enum cros_gpio_index index;
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

int cros_gpio_fetch(enum cros_gpio_index index, cros_gpio_t *gpio);

#ifdef VBOOT_DEBUG
int cros_gpio_dump(cros_gpio_t *gpio);
#else
static int cros_gpio_dump(cros_gpio_t *gpio) { return 0; }
#endif

#endif /* CROS_GPIO_H__ */
