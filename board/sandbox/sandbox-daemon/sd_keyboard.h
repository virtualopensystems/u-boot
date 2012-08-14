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
#ifndef __SD_KEYBOARD_H
#define __SD_KEYBOARD_H

#include "asm/sandbox-api.h"

/*
 * Backing file pathname.
 * Backing file contains characters to be produced by the keyboard.
 */
extern char *keyboard_file;

/**
 * Initializes the keyboard system.
 *
 * @param db	Pointer to doorbell data
 */
void keyboard_initialize(struct doorbell_t *db);

/**
 * Executes the requested keyboard command.
 *
 * @param dbc	Pointer to doorbell command data
 */
void keyboard_command(struct doorbell_command_t *dbc);
#endif
