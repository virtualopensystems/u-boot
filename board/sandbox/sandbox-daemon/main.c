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
#include "sd_spi.h"
#include "asm/sandbox-api.h"


static void help(const char *program_name)
{
	fprintf(stderr,
		"\n"
		"%s\n\t["
		"--help | --verbose\n"
		"\t| --spi-page-size | --spi-page-count | "
		"--spi-vendor | --spi-file\n"
		"\t]..."
		"\n\n"
		"The sandbox daemon manages the shared memory "
		"region used by the\nsandbox version of u-boot."
		"\n\n"
		"  --help:            Show this help message and exit\n"
		"  --verbose:         Turn on verbosity\n"
		"  --spi-page-size:   SPI ROM page size in bytes\n"
		"  --spi-page-count:  SPI ROM total page count\n"
		"  --spi-page-vendor: SPI ROM vendor name\n"
		"  --spi-page-file:   SPI ROM backing file\n"
		"\n", program_name);
	exit(0);
}

static void process_args(int argc, char * const argv[])
{
	/* TODO(thutt@chromium.org):
	 *
	 * Redesign the method used to provide device information.
	 * Having to supply information about each device will become
	 * cumbersome when there are a lot of drivers avaialble.
	 * Perhaps having a more complicated backing file scheme where
	 * the device information is embedded in the backing file;
	 * only supplying the name to the backing file would be
	 * necessary then.
	 */
	struct option args[] = {
		{ "help",	     no_argument,	NULL,	256 },
		{ "verbose",	     no_argument,	NULL,	257 },
		{ "spi-page-size",   required_argument, NULL,	258 },
		{ "spi-page-count",  required_argument, NULL,	259 },
		{ "spi-vendor",	     required_argument, NULL,	260 },
		{ "spi-file",	     required_argument, NULL,	261 },
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
			help(argv[0]);
			break;

		case 'v':
		case 257:
			arg_verbose = 1;
			verbose("verbosity enabled\n");
			break;

		case 258:
			spi_page_size = strtol(optarg, NULL, 0);
			break;

		case 259:
			spi_page_count = strtol(optarg, NULL, 0);
			break;

		case 260:
			spi_vendor = strdup(optarg);
			break;

		case 261:
			spi_file = strdup(optarg);
			break;

		default:
			help(argv[0]);
			break;
		}
	}

	if (!validate_spi_arguments())
		fatal("SPI peripheral not configured correctly");

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
