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

#include <fdtdec.h>

enum vboot_flag_id {
	VBOOT_FLAG_WRITE_PROTECT = 0,
	VBOOT_FLAG_RECOVERY,
	VBOOT_FLAG_DEVELOPER,
	VBOOT_FLAG_LID_OPEN,
	VBOOT_FLAG_POWER_OFF,

	VBOOT_FLAG_MAX_FLAGS
};

/* The struct to store the context for each flag */
struct vboot_flag_context {
	struct vboot_flag_driver *driver;	/* the driver for ths eflag */
	int config_node;	/* offset of the config node in fdt */
	int node;		/* offset of the vboot flag node in fdt */
};

/* GPIO details required in the crossystem data structure */
struct vboot_flag_details {
	int value;		/* the value of the flag, either 1 or 0 */
	int port;		/* the gpio port number */
	int active_high;	/* 1 if the port is active high; 0 active low */
};

/* API function pointers of the driver to get vboot flag info */
struct vboot_flag_driver {
	/* the fdt compatible type */
	enum fdt_compat_id type;
	/**
	 * The setup() function will be call in vboot_flag_init()
	 *
	 * @prarm id		ID of VBoot flag
	 * @param context	Pointer to the context which keeps internal data
	 * @return zero on success and non-zero on failures
	 */
	int (*setup)(enum vboot_flag_id id, struct vboot_flag_context *context);
	/**
	 * The fetch() function returns the details of the vboot flag
	 *
	 * @prarm id		ID of VBoot flag
	 * @param context	Pointer to the context which keeps internal data
	 @ @param details	Returns the details of the flag
	 * @return zero on success and non-zero on failures
	 */
	int (*fetch)(enum vboot_flag_id id, struct vboot_flag_context *context,
			struct vboot_flag_details *details);
};

/**
 * Get the node name of the vboot flag
 *
 * @prarm id		ID of VBoot flag
 * @return the node name string for the flag
 */
const char *vboot_flag_node_name(enum vboot_flag_id id);

/**
 * Initialize vboot_flag module
 *
 * @return zero on success and non-zero on failures
 */
int vboot_flag_init(void);

/**
 * Fetch the details of the required vboot flag
 *
 * @param id		ID of VBoot flag
 * @param details	Returns the GPIO details
 * @return zero on success and non-zero on failures
 */
int vboot_flag_fetch(enum vboot_flag_id id, struct vboot_flag_details *details);

/* TODO(waihong) Remove the legacy below when vboot flag refactoring done */

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
