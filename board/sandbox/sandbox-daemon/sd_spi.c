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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"
#include "doorbell-command.h"
#include "asm/sandbox-api.h"
#include "sd_spi.h"

__u32 spi_page_size;		/* page size in bytes */
__u32 spi_page_count;		/* number of pages */
char *spi_vendor;
char *spi_file;

int validate_spi_arguments(void)
{
	unsigned error = 0;
	/*
	 * If a vendor for the SPI has not been specified, then there
	 * is no SPI ROM in this run.
	 */
	if (spi_vendor == NULL)
		return 0;

	if (spi_page_size == 0) {
		fprintf(stderr, "--spi-page-size cannot be zero.\n");
		++error;
	}

	if (spi_page_count == 0) {
		fprintf(stderr, "--spi-page-count cannot be zero.\n");
		++error;
	}

	if (spi_vendor == NULL) {
		fprintf(stderr, "--spi-vencor was not set.\n");
		++error;
	}

	if (spi_file == NULL) {
		fprintf(stderr, "--spi-file was not set.\n");
		++error;
	}
	return error;
}

void initialize_spi(struct doorbell_t *db)
{
	if (spi_vendor == NULL)
		return;		/* No SPI enabled */

	db->spi.page_size = spi_page_size;
	db->spi.n_pages	  = spi_page_count;
	strncpy(&db->spi.vendor[0], spi_vendor,
		sizeof(db->spi.vendor) / sizeof(db->spi.vendor[0]));
}

/* open_spi_file: Open SPI file.  Creates file if it does not exist. */
static int open_spi_file(void)
{
	if (spi_file == NULL)
		return -1;

	return open(spi_file, O_RDWR | O_CREAT, 0600);
}

static unsigned spi_size(void)
{
	return spi_page_size * spi_page_count;
}

static unsigned spi_out_of_bounds(struct doorbell_command_t *dbc,
				  unsigned offset,
				  unsigned len)
{
	if (offset + len > spi_size()) {
		command_failure(dbc, SB_SPI);
		return 1;
	}
	return 0;
}

static void spi_read(struct doorbell_command_t *dbc,
		     int fd,
		     unsigned offset,
		     unsigned len,
		     void *buf)
{
	off_t s = lseek(fd, offset, SEEK_SET);

	if (spi_out_of_bounds(dbc, offset, len))
		return;

	if (s == (off_t)-1) {
		/* Unable to seek. Return all-bits-set. */
		memset(buf, 0xff, len);
	} else {
		ssize_t s = read(fd, buf, len);
		if (s == -1) {
			command_failure(dbc, SB_SPI);
		} else if (s < (ssize_t)len) {
			/* A partial read has occurred.
			 *
			 * It is an in-bound read of the SPI, because
			 * we passed the bounds check above.
			 *
			 * The first 'len' bytes were read, but the
			 * file associated with the SPI has not been
			 * written with data yet.  We treat the
			 * remaining bytes as uninitialized and set
			 * them to 0xff.
			 */
			memset(buf + s, 0xff, (ssize_t)len - s);
		}
	}
}

static void spi_write(struct doorbell_command_t *dbc,
		      int fd,
		      unsigned offset,
		      unsigned len,
		      void *buf)
{
	off_t s = lseek(fd, offset, SEEK_SET);

	if (spi_out_of_bounds(dbc, offset, len))
		return;

	if (s == (off_t)-1) { /* Unable to seek? */
		command_failure(dbc, SB_SPI);
	} else {
		ssize_t s = write(fd, buf, len);

		if (s < (ssize_t)len) {
			/* Error reading file, or a full read was not
			 * completed.  Fail the command.
			 */
			command_failure(dbc, SB_SPI);
		}
	}
}

static void spi_erase(struct doorbell_command_t *dbc,
		      int fd,
		      unsigned offset,
		      unsigned len)
{
	unsigned char buf[256];
	unsigned      i;

	if (spi_out_of_bounds(dbc, offset, len))
		return;

	memset(buf, '\0', sizeof(buf));

	i = 0;
	while (dbc->result == 0 && i < len) {
		unsigned b = sizeof(buf);

		if (i + sizeof(buf) >= len)
			b = len - i;

		spi_write(dbc, fd, offset, b, buf);
		offset += b;
		i      += sizeof(buf);
	}
}

static void spi_unknown_command(struct doorbell_command_t *dbc,
				unsigned command)
{
	fprintf(stderr, "Unhandled SPI command '%u'\n", command);
	command_failure(dbc, SB_SPI);
}

void spi_command(struct doorbell_command_t *dbc)
{
	int fd;
	unsigned command = dbc->command_data[0];
	unsigned offset	 = dbc->command_data[1];
	unsigned len = dbc->command_data[2];
	void *buf = (void *)(uintptr_t)dbc->command_data[3];

	verbose("SPI command: [%#x, %#x, %#x, %p]\n",
		command, offset, len, buf);

	fd = open_spi_file();
	if (fd == -1) {
		fprintf(stderr, "Unable to open/create '%s'\n", spi_file);
		command_failure(dbc, SB_SPI);
		return;
	}

	switch (command) {
	case 0:
		spi_read(dbc, fd, offset, len, buf);
		break;

	case 1:
		spi_write(dbc, fd, offset, len, buf);
		break;

	case 2:
		spi_erase(dbc, fd, offset, len);
		break;

	default:
		spi_unknown_command(dbc, command);
		break;
	}

	close(fd);
}
