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
#ifndef __PROCESS_MEMORY_H
#define __PROCESS_MEMORY_H

/**
 * Process shared memory.
 *
 *   This function watches the shared memory area for 'device access'
 *   and responds by fulfilling the device request.
 *
 *   For memory-mapped devices, the function will watch specific
 *   memory locations associated with the device.  When the monitored
 *   memory locations are changed, the daemon will respond.
 *
 *   For devices which are not memory mapped, the doorbell area will
 *   be monitored for device access.  When the doorbell is set, the
 *   deamon will disptach the request to the device that 'rang' the
 *   doorbell.
 */
void process_memory(void);
#endif
