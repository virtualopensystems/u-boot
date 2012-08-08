/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __SHARED_MEMORY_H
#define __SHARED_MEMORY_H

/* TODO(thutt): Provide a standardized naming convention. */

/**
 * Remove the shared memory region
 *
 *  If the shared memory region exists, it is removed.  If it does not
 *  exist, nothing happens.
 */
void remove_shared_memory(void);

/**
 * Initializes the shared memory region
 *
 *   Creates & attaches to the shared memory region.  The region
 *   identifier is denoted by SANDBOX_SHM_KEY, and the size by
 *   SANDBOX_SHM_SIZE.
 *
 *   If the area cannot be allocated, or cannot be attached, it is a
 *   fatal error and this function will kill the process with an
 *   appropriate message.
 */
void initialize_shared_memory(void);

/**
 * Initializes the shared memory region devices
 *
 *   Calls the initialization function for the devices that are
 *   supported by the sandbox.
 *
 *   This function can only be performed after the shared memory is
 *   initialized and after the command line arguments are processed.
 *
 *   If a device's initialization code cannot find
 *   command-line-supplied infomation, or if it is erroneous, the
 *   device will simply not exist in the Sandbox U-Boot system.
 */
void initialize_shared_devices(void);
#endif
