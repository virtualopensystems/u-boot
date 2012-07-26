/*
 * Copyright (c) 2012 The Chromium OS Authors.
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

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include "asm/sandbox-api.h"

static int read(struct spi_flash *flash, u32 offset, size_t len, void *buf)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->device_id = SB_SPI;
	dbc->command_data[0] = 0; /* read */
	dbc->command_data[1] = offset;
	dbc->command_data[2] = len;
	dbc->command_data[3] = (__u32)(uintptr_t)buf;
	sandbox_ring_doorbell();
	return dbc->result;
}

static int write(struct spi_flash *flash, u32 offset,
		 size_t len, const void *buf)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->device_id = SB_SPI;
	dbc->command_data[0] = 1; /* write */
	dbc->command_data[1] = offset;
	dbc->command_data[2] = len;
	dbc->command_data[3] = (__u32)(uintptr_t)buf;
	sandbox_ring_doorbell();
	return dbc->result;
}

static int erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct doorbell_command_t *dbc = sandbox_get_doorbell_command();

	dbc->device_id = SB_SPI;
	dbc->command_data[0] = 2; /* erase */
	dbc->command_data[1] = offset;
	dbc->command_data[2] = len;
	sandbox_ring_doorbell();
	return dbc->result;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *sf;
	struct doorbell_t *db = sandbox_get_doorbell();
	struct spi_t *spi = &db->spi;
	char *name;

	if (spi->vendor[0] == '\0') /* No SPI device */
	    return NULL;

	sf = malloc(sizeof(struct spi_flash));
	if (sf == NULL)
		return NULL;

	name = malloc(sizeof(spi->vendor) + 1);
	if (name == NULL) {
		free(sf);
		return NULL;
	}

	sf->name = name;
	sf->spi = NULL;
	sf->size = spi->page_size * spi->n_pages;
	sf->page_size = spi->page_size;
	sf->sector_size = 512;
	sf->read = read;
	sf->write = write;
	sf->erase = erase;
	strncpy(name, spi->vendor, sizeof(spi->vendor));
	name[sizeof(spi->vendor)] = '\0';

	printf("SF: Detected %s with page size ", sf->name);
	print_size(sf->sector_size, ", total ");
	print_size(sf->size, "\n");
	return sf;
}

void spi_flash_free(struct spi_flash *flash)
{
	free((void *)flash->name);
	free(flash);
}

