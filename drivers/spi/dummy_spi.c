/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>

#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	printf("spi_cs_is_valid used but not implemented.\n");
	return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	printf("spi_setup_slave used but not implemented.\n");
	return NULL;
}

void spi_free_slave(struct spi_slave *slave)
{
	printf("spi_free_slave used but not implemented.\n");
}

void spi_init(void)
{
	printf("spi_init used but not implemented.\n");
}

int spi_claim_bus(struct spi_slave *slave)
{
	printf("spi_claim_bus used but not implemented.\n");
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	printf("spi_release_bus used but not implemented.\n");
}

void spi_cs_activate(struct spi_slave *slave)
{
	printf("spi_cs_activate used but not implemented.\n");
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	printf("spi_cs_deactivate used but not implemented.\n");
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	printf("spi_xfer used but not implemented.\n");
	return 0;
}
