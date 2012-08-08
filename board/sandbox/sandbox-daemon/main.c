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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "doorbell-command.h"
#include "lib.h"
#include "process-memory.h"
#include "shared-memory.h"
#include "asm/sandbox-api.h"


static void help(void)
{
	fprintf(stderr,
		"\n"
		"sandbox-daemon "
		"[--help | --verbose]..."
		"\n\n"
		"The sandbox daemon manages the shared memory "
		"region used by the\nsandbox version of u-boot."
		"\n\n"
		"  --help:    Show this help message and exit\n"
		"  --verbose: Turn on verbosity\n"
		"\n");
	exit(0);
}

static void process_args(int argc, char * const argv[])
{
	struct option args[] = {
		{ "help",	     no_argument,	NULL,	256 },
		{ "verbose",	     no_argument,	NULL,	257 },
		{ NULL,		     no_argument,	NULL,	  0 }
	};

	while (1) {
		int index;
		const int c = getopt_long(argc, argv, "hv", args, &index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
		case 256:
			help();
			break;

		case 'v':
		case 257:
			arg_verbose = 1;
			verbose("verbosity enabled\n");
			break;

		default:
			help();
			break;
		}
	}
}

int main(int argc, char * const argv[])
{
	pid_t child;

	initialize_shared_memory(); /* process_args() uses shared memory. */
	process_args(argc, argv);
	initialize_shared_devices();

	child = fork();
	if (child == 0) {
		/* running in child context */
		process_memory();
	} else
		verbose("Daemonized process id: %u\n", child);

	return 0;
}
