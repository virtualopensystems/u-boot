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
#include <stdio.h>

#include "lib.h"
#include "asm/sandbox-api.h"
#include "doorbell-command.h"
#include "sd_keyboard.h"

char *keyboard_file;
FILE *fp;
long  keyboard_length;

void keyboard_initialize(struct doorbell_t *db)
{
	if (keyboard_file) {
		fp = fopen(keyboard_file, "r");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			keyboard_length = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		} else
			fprintf(stderr, "Unable to open '%s'\n", keyboard_file);
	}
}

/**
 * Checks if a key press is available.
 *
 * @param dbc	Pointer to doorbell command data
 * Result == 0 -> no key press available
 * Result != 0 -> key press available
 */
static void keyboard_tstc(struct doorbell_command_t *dbc)
{
	dbc->dbc_buf[0] = (fp != NULL &&
			   ftell(fp) < keyboard_length);
}

/**
 * Reads a key from the keyboard backing file.
 *
 * @param dbc	Pointer to doorbell command data
 *
 *  This function should only be called if keyboard_tstc() indicates
 *  that a keypress is available.
 *
 *  If no keypress is available, this function does nothing.
 *
 *  If a keypress is available, this dbc->dbc_buf[0] holds the 8-bit
 *  character.
 */
static void keyboard_getc(struct doorbell_command_t *dbc)
{
	const size_t n_chars = 1;

	dbc->dbc_buf[0] = 0;
	if (fp != NULL && ftell(fp) < keyboard_length)
		fread(&dbc->dbc_buf[0], sizeof(char), n_chars, fp);
}

void keyboard_command(struct doorbell_command_t *dbc)
{
	unsigned command = dbc->command_data[0];

	switch (command) {
	case 0:
		keyboard_tstc(dbc);
		break;
	case 1:
		keyboard_getc(dbc);
		break;
	}
}
