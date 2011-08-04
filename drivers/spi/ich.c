/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This file is derived from the flashrom project. */

#include <common.h>
#include <malloc.h>
#include <pci.h>
#include <pci_ids.h>

#include <spi.h>

#include <asm/io.h>

typedef struct spi_slave ich_spi_slave;

static int ichspi_lock = 0;

typedef struct ich7_spi_regs {
	uint16_t spis;
	uint16_t spic;
	uint32_t spia;
	uint64_t spid[8];
	uint64_t _pad;
	uint32_t bbar;
	uint16_t preop;
	uint16_t optype;
	uint64_t opmenu;
} __attribute__((packed)) ich7_spi_regs;

static ich7_spi_regs *ich7_spi = NULL;

enum {
	SPIS_SCIP =		0x0001,
	SPIS_GRANT =		0x0002,
	SPIS_CDS =		0x0004,
	SPIS_FCERR =		0x0008,
	SPIS_LOCK =		0x8000,
	SPIS_RESERVED_MASK =	0x7ff0
};

enum {
	SPIC_SCGO =		0x0002,
	SPIC_ACS =		0x0004,
	SPIC_SPOP =		0x0008,
	SPIC_DS =		0x4000
};

enum {
	SPI_OPCODE_TYPE_READ_NO_ADDRESS =	0,
	SPI_OPCODE_TYPE_WRITE_NO_ADDRESS =	1,
	SPI_OPCODE_TYPE_READ_WITH_ADDRESS =	2,
	SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS =	3
};

static void write_reg(const void *value, void *dest, uint32_t size)
{
	const uint8_t *bvalue = value;
	uint8_t *bdest = dest;

	while (size >= 4) {
		writel(*(const uint32_t *)bvalue, bdest);
		bdest += 4; bvalue += 4; size -= 4;
	}
	while (size) {
		writeb(*bvalue, bdest);
		bdest++; bvalue++; size--;
	}
}

static void read_reg(const void *src, void *value, uint32_t size)
{
	const uint8_t *bsrc = src;
	uint8_t *bvalue = value;

	while (size >= 4) {
		*(uint32_t *)bvalue = readl(bsrc);
		bsrc += 4; bvalue += 4; size -= 4;
	}
	while (size) {
		*bvalue = readb(bsrc);
		bsrc++; bvalue++; size--;
	}
}

static void ich_set_bbar(uint32_t minaddr)
{
	const uint32_t bbar_mask = 0x00ffff00;
	uint32_t ichspi_bbar;

	minaddr &= bbar_mask;
	ichspi_bbar = readl(&ich7_spi->bbar) & ~bbar_mask;
	ichspi_bbar |= minaddr;
	writel(ichspi_bbar, &ich7_spi->bbar);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	puts("spi_cs_is_valid used but not implemented\n");
	return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	ich_spi_slave *slave = malloc(sizeof(*slave));

	if (!slave) {
		puts("ICH SPI: Bad allocation\n");
		return NULL;
	}

	memset(slave, 0, sizeof(*slave));
	slave->bus = bus;
	slave->cs = cs;
	return (struct spi_slave *)slave;
}

void spi_free_slave(struct spi_slave *_slave)
{
	ich_spi_slave *slave = (ich_spi_slave *)_slave;
	free(slave);
}

void spi_init(void)
{
	const uint16_t spibar_offset = 0x3020;
	uint8_t *rcrb; /* Root Complex Register Block */
	uint32_t rcba; /* Root Complex Base Address */
	uint8_t bios_cntl;

	pci_dev_t dev = pci_find_device(PCI_VENDOR_ID_INTEL,
					PCI_DEVICE_ID_INTEL_TGP_LPC, 0);
	/* The docs say it should be at device 31, function 0. */
	if (PCI_DEV(dev) != 31 || PCI_FUNC(dev) != 0) {
		puts("ICH SPI: LPC bridge not where it's supposed to be\n");
		return;
	}
	pci_read_config_dword(dev, 0xf0, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable. */
	rcrb = (uint8_t *)(rcba & 0xffffc000);
	ich7_spi = (ich7_spi_regs *)(rcrb + spibar_offset);

	ichspi_lock = readw(&ich7_spi->spis) & SPIS_LOCK;
	ich_set_bbar(0);

	/* Disable the BIOS write protect so write commands are allowed. */
	pci_read_config_byte(dev, 0xdc, &bios_cntl);
	pci_write_config_byte(dev, 0xdc, bios_cntl | 0x1);
}

int spi_claim_bus(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
}

typedef struct spi_transaction {
	const uint8_t *out;
	uint32_t bytesout;
	uint8_t *in;
	uint32_t bytesin;
	uint8_t type;
	uint8_t opcode;
	uint32_t offset;
} spi_transaction;

static inline void spi_use_out(spi_transaction *trans, unsigned bytes)
{
	trans->out += bytes;
	trans->bytesout -= bytes;
}

static inline void spi_use_in(spi_transaction *trans, unsigned bytes)
{
	trans->in += bytes;
	trans->bytesin -= bytes;
}

static void spi_setup_type(spi_transaction *trans)
{
	trans->type = 0xFF;

	/* Try to guess spi type from read/write sizes. */
	if (trans->bytesin == 0)
		/* If bytesin = 0 and bytesout >= 4, we don't know if
		 * it's WRITE_NO_ADDRESS or WRITE_WITH_ADDRESS. If we
		 * use WRITE_NO_ADDRESS and the first 3 data bytes are
		 * actual the address, they go to the bus anyhow.
		 */
		trans->type = SPI_OPCODE_TYPE_WRITE_NO_ADDRESS;
	else if (trans->bytesout == 1) /* and bytesin is > 0 */
		trans->type = SPI_OPCODE_TYPE_READ_NO_ADDRESS;
	else if (trans->bytesout == 4) /* and bytesin is > 0 */
		trans->type = SPI_OPCODE_TYPE_READ_WITH_ADDRESS;
}

static int spi_setup_opcode(spi_transaction *trans)
{
	uint16_t optypes;
	uint8_t opmenu[8];

	trans->opcode = trans->out[0];
	spi_use_out(trans, 1);
	if (!ichspi_lock) {
		/* The lock is off, so just use index 0. */
		writeb(trans->opcode, &ich7_spi->opmenu);
		optypes = readw(&ich7_spi->optype);
		optypes = (optypes & 0xfffc) | (trans->type & 0x3);
		writew(optypes, &ich7_spi->optype);
		return 0;
	} else {
		/* The lock is on. See if what we need is on the menu. */
		uint8_t optype;
		uint16_t opcode_index;

		read_reg(&ich7_spi->opmenu, &opmenu, sizeof(opmenu));
		for (opcode_index = 0; opcode_index < ARRAY_SIZE(opmenu);
				opcode_index++) {
			if (opmenu[opcode_index] == trans->opcode)
				break;
		}

		if (opcode_index == ARRAY_SIZE(opmenu)) {
			printf("ICH SPI: Opcode %x not found\n",
				trans->opcode);
			return -1;
		}

		optypes = readw(&ich7_spi->optype);
		optype = (optypes >> (opcode_index * 2)) & 0x3;
		if (trans->type == SPI_OPCODE_TYPE_WRITE_NO_ADDRESS &&
			optype == SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS &&
			trans->bytesout >= 3) {
			/* We guessed wrong earlier. Fix it up. */
			trans->type = optype;
		}
		if (optype != trans->type) {
			printf("ICH SPI: Transaction doesn't fit type %d\n",
				optype);
			return -1;
		}
		return opcode_index;
	}
}

static int spi_setup_offset(spi_transaction *trans)
{
	/* Separate the SPI address and data. */
	switch (trans->type) {
	case SPI_OPCODE_TYPE_READ_NO_ADDRESS:
	case SPI_OPCODE_TYPE_WRITE_NO_ADDRESS:
		return 0;
	case SPI_OPCODE_TYPE_READ_WITH_ADDRESS:
	case SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS:
		trans->offset = ((uint32_t)trans->out[0] << 16) |
				((uint32_t)trans->out[1] << 8) |
				((uint32_t)trans->out[2] << 0);
		spi_use_out(trans, 3);
		return 1;
	default:
		printf("Unrecognized SPI transaction type %#x\n", trans->type);
		return -1;
	}
}

int spi_xfer(struct spi_slave *slave, const void *dout, unsigned int bitsout,
		void *din, unsigned int bitsin)
{
	int timeout;

	uint16_t spis, spic;
	int16_t opcode_index;
	int with_address;

	spi_transaction trans = {
		dout, bitsout / 8,
		din, bitsin / 8,
		0xff, 0xff, 0
	};

	/* There has to always at least be an opcode. */
	if (!bitsout || !dout) {
		puts("ICH SPI: No opcode for transfer\n");
		return 1;
	}
	/* Make sure if we read something we have a place to put it. */
	if (bitsin != 0 && !din) {
		puts("ICH SPI: Read but no target buffer\n");
		return 1;
	}
	/* Right now we don't support writing partial bytes. */
	if (bitsout % 8 || bitsin % 8) {
		puts("ICH SPI: Accessing partial bytes not supported\n");
		return 1;
	}

	/* 60 ms are 9.6 million cycles at 16 MHz. */
	timeout = 100 * 60;
	while ((readw(&ich7_spi->spis) & SPIS_SCIP) && --timeout)
		udelay(10);
	if (!timeout) {
		puts("ICH SPI: SCIP never cleared\n");
		return 1;
	}

	spi_setup_type(&trans);
	if ((opcode_index = spi_setup_opcode(&trans)) < 0)
		return 1;
	if ((with_address = spi_setup_offset(&trans)) < 0)
		return 1;

	/*
	 * Read or write up to 64 bytes at a time until everything has been
	 * sent.
	 */
	while (trans.bytesout || trans.bytesin) {
		const unsigned maxdata = 64;
		uint32_t data_length;

		/* SPI addresses are 24 bit only */
		writel(trans.offset & 0x00FFFFFF, &ich7_spi->spia);

		if (trans.bytesout)
			data_length = min(trans.bytesout, maxdata);
		else
			data_length = min(trans.bytesin, maxdata);

		/* Program data into FDATA0 to N */
		if (trans.bytesout) {
			write_reg(trans.out, ich7_spi->spid, data_length);
			spi_use_out(&trans, data_length);
			if (with_address)
				trans.offset += data_length;
		}

		/* Assemble SPIS */
		spis = readw(&ich7_spi->spis);
		/* keep reserved bits */
		spis &= SPIS_RESERVED_MASK;
		/* clear error status registers */
		spis |= (SPIS_CDS | SPIS_FCERR);
		writew(spis, &ich7_spi->spis);

		/* Assemble SPIC */
		spic = (opcode_index & 0x07) << 4;

		if (data_length != 0) {
			spic |= SPIC_DS;
			spic |= (data_length - 1) << 8;
		}

		timeout = 100 * 60;

		/* Start */
		spic |= SPIC_SCGO;

		/* write it */
		writew(spic, &ich7_spi->spic);

		/* Wait for Cycle Done Status or Flash Cycle Error. */
		spis = readw(&ich7_spi->spis);
		while (((spis & (SPIS_CDS | SPIS_FCERR)) == 0) && --timeout) {
			udelay(10);
			spis = readw(&ich7_spi->spis);
		}
		if (!timeout) {
			printf("timeout, ICH7_REG_SPIS=0x%04x\n",
				readw(&ich7_spi->spis));
			return 1;
		}

		if (spis & SPIS_FCERR) {
			puts("ICH SPI: Transaction error\n");
			/* keep reserved bits */
			spis &= SPIS_RESERVED_MASK;
			writew(spis | SPIS_FCERR, &ich7_spi->spis);
			return 1;
		}

		if (trans.bytesin) {
			read_reg(ich7_spi->spid, trans.in, data_length);
			spi_use_in(&trans, data_length);
			if (with_address)
				trans.offset += data_length;
		}
	}
	return 0;
}
