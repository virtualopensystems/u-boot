/*
 * Gigadevice SPI flash driver
 * Copyright 2012, Samsung Electronics Co., Ltd.
 * Author: Banajit Goswami <banajit.g@samsung.com>
 *
 * Based on:
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* GD25Pxx-specific commands */
#define CMD_GD25_SE		0x20	/* Sector (4K) Erase */

struct gigadevice_spi_flash_params {
	uint16_t	id;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct gigadevice_spi_flash_params gigadevice_spi_flash_table[] = {
	{
		.id			= 0x6016,
		.nr_blocks		= 64,
		.name			= "GD25LQ",
	},

};

static int gigadevice_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_GD25_SE, offset, len);
}

struct spi_flash *spi_flash_probe_gigadevice(struct spi_slave *spi, u8 *idcode)
{
	const struct gigadevice_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;
	unsigned page_size;

	for (i = 0; i < ARRAY_SIZE(gigadevice_spi_flash_table); i++) {
		params = &gigadevice_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(gigadevice_spi_flash_table)) {
		debug("SF: Unsupported Gigadevice ID %02x%02x\n",
				idcode[1], idcode[2]);
		return NULL;
	}

	flash = malloc(sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->spi = spi;
	flash->name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = 256;

	flash->write = spi_flash_cmd_write_multi;
	flash->erase = gigadevice_erase;
#ifdef CONFIG_SPI_FLASH_NO_FAST_READ
	flash->read = spi_flash_cmd_read_slow;
#else
	flash->read = spi_flash_cmd_read_fast;
#endif
	flash->page_size = page_size;
	/* sector_size = page_size * pages_per_sector */
	flash->sector_size = page_size * 16;
	/* size = sector_size * sector_per_block * number of blocks */
	flash->size = flash->sector_size * 16 * params->nr_blocks;

	return flash;
}
