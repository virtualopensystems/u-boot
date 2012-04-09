/*
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same I2C controller inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>

#include <asm/io.h>
#include <i2c.h>
#include "s3c24x0_i2c.h"

/* TODO(crosbug.com/26861): Add back support of S3C24X0 via device tree. */

#ifdef CONFIG_HARD_I2C

#define I2C_WRITE	0
#define I2C_READ	1

#define I2C_OK		0
#define I2C_NOK		1
#define I2C_NACK	2
#define I2C_NOK_LA	3	/* Lost arbitration */
#define I2C_NOK_TOUT	4	/* time out */

#define I2CSTAT_BSY	0x20	/* Busy bit */
#define I2CSTAT_NACK	0x01	/* Nack bit */
#define I2CCON_ACKGEN	0x80	/* Acknowledge generation */
#define I2CCON_IRPND	0x10	/* Interrupt pending bit */
#define I2C_MODE_MT	0xC0	/* Master Transmit Mode */
#define I2C_MODE_MR	0x80	/* Master Receive Mode */
#define I2C_START_STOP	0x20	/* START / STOP */
#define I2C_TXRX_ENA	0x10	/* I2C Tx/Rx enable */

#define I2C_TIMEOUT	1	/* 1 second */

/*
 * This is needed here, as i2c is build as a part of SPL,
 * and during the i2c clock calculation its actually looks for the s5p_cpu_id
 */
#ifdef CONFIG_SPL_BUILD
unsigned int s5p_cpu_id = 0xc520;
#endif

/* We should not rely on any particular ordering of these IDs */
static enum periph_id periph_for_dev[] = {
	PERIPH_ID_I2C0,
	PERIPH_ID_I2C1,
	PERIPH_ID_I2C2,
	PERIPH_ID_I2C3,
	PERIPH_ID_I2C4,
	PERIPH_ID_I2C5,
	PERIPH_ID_I2C6,
	PERIPH_ID_I2C7,
};

static unsigned int g_current_bus;

static enum periph_id i2c_get_periph_id(unsigned dev_index)
{
	if (dev_index < ARRAY_SIZE(periph_for_dev))
		return periph_for_dev[dev_index];
	debug("%s: invalid bus %d", __func__, dev_index);
	return PERIPH_ID_NONE;
}

static struct s3c24x0_i2c *get_base_i2c(int bus_idx)
{
	struct s3c24x0_i2c *i2c = (struct s3c24x0_i2c *)samsung_get_base_i2c();
	return &i2c[bus_idx];
}

static inline struct exynos5_gpio_part1 *exynos_get_base_gpio1(void)
{
	return (struct exynos5_gpio_part1 *)(EXYNOS5_GPIO_PART1_BASE);
}

static int WaitForXfer(struct s3c24x0_i2c *i2c)
{
	int i;

	i = I2C_TIMEOUT * 10;
	while (!(readl(&i2c->iiccon) & I2CCON_IRPND) && (i > 0)) {
		udelay(100);
		i--;
	}

	return (readl(&i2c->iiccon) & I2CCON_IRPND) ? I2C_OK : I2C_NOK_TOUT;
}

static int IsACK(struct s3c24x0_i2c *i2c)
{
	return !(readl(&i2c->iicstat) & I2CSTAT_NACK);
}

static void ReadWriteByte(struct s3c24x0_i2c *i2c)
{
	writel(readl(&i2c->iiccon) & ~I2CCON_IRPND, &i2c->iiccon);
}

static void i2c_ch_init(struct s3c24x0_i2c *i2c, int speed, int slaveadd)
{
	ulong freq, pres = 16, div;

	freq = get_i2c_clk();
	/* calculate prescaler and divisor values */
	if ((freq / pres / (16 + 1)) > speed)
		/* set prescaler to 512 */
		pres = 512;

	div = 0;
	while ((freq / pres / (div + 1)) > speed)
		div++;

	/* set prescaler, divisor according to freq, also set ACKGEN, IRQ */
	writel((div & 0x0F) | 0xA0 | ((pres == 512) ? 0x40 : 0), &i2c->iiccon);

	/* init to SLAVE REVEIVE and set slaveaddr */
	writel(0, &i2c->iicstat);
	writel(slaveadd, &i2c->iicadd);
	/* program Master Transmit (and implicit STOP) */
	writel(I2C_MODE_MT | I2C_TXRX_ENA, &i2c->iicstat);
}

/*
 * MULTI BUS I2C support
 */
#ifdef CONFIG_EXYNOS5
static void i2c_bus_init(struct s3c24x0_i2c *i2c, unsigned int bus)
{
	struct exynos5_gpio_part1 *gpio;
	gpio = exynos_get_base_gpio1();

	int periph_id = i2c_get_periph_id(bus);

	exynos_pinmux_config(periph_id, 0);

	i2c_ch_init(i2c, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
}
#else
static void i2c_bus_init(struct s3c24x0_i2c *i2c, unsigned int bus) {}
#endif

#ifdef CONFIG_I2C_MULTI_BUS
int i2c_set_bus_num(unsigned int bus)
{
	struct s3c24x0_i2c *i2c;

	if ((bus < 0) || (bus >= CONFIG_MAX_I2C_NUM)) {
		debug("Bad bus: %d\n", bus);
		return -1;
	}

	g_current_bus = bus;
	i2c = get_base_i2c(g_current_bus);
	i2c_bus_init(i2c, g_current_bus);

	return 0;
}

unsigned int i2c_get_bus_num(void)
{
	return g_current_bus;
}
#endif

void i2c_init(int speed, int slaveadd)
{
	struct s3c24x0_i2c *i2c;
	struct exynos5_gpio_part1 *gpio;
	int i;

	/* By default i2c channel 0 is the current bus */
	g_current_bus = I2C0;

	i2c = get_base_i2c(g_current_bus);

	i2c_bus_init(i2c, g_current_bus);

	/* wait for some time to give previous transfer a chance to finish */
	i = I2C_TIMEOUT * 1000;
	while ((readl(&i2c->iicstat) & I2CSTAT_BSY) && (i > 0)) {
		udelay(1000);
		i--;
	}

	gpio = exynos_get_base_gpio1();
	writel((readl(&gpio->b3.con) & ~0x00FF) | 0x0022, &gpio->b3.con);

	i2c_ch_init(i2c, speed, slaveadd);
}

/*
 * cmd_type is 0 for write, 1 for read.
 *
 * addr_len can take any value from 0-255, it is only limited
 * by the char, we could make it larger if needed. If it is
 * 0 we skip the address write cycle.
 */
static int i2c_transfer(struct s3c24x0_i2c *i2c,
			unsigned char cmd_type,
			unsigned char chip,
			unsigned char addr[],
			unsigned char addr_len,
			unsigned char data[],
			unsigned short data_len)
{
	int i, result;

	if (data == 0 || data_len == 0) {
		/* Don't support data transfer of no length or to address 0 */
		debug("i2c_transfer: bad call\n");
		return I2C_NOK;
	}

	/* Check I2C bus idle */
	i = I2C_TIMEOUT * 10;
	while ((readl(&i2c->iicstat) & I2CSTAT_BSY) && (i > 0)) {
		udelay(1000);
		i--;
	}

	if (readl(&i2c->iicstat) & I2CSTAT_BSY)
		return I2C_NOK_TOUT;

	writel(readl(&i2c->iiccon) | I2CCON_ACKGEN, &i2c->iiccon);
	result = I2C_OK;

	switch (cmd_type) {
	case I2C_WRITE:
		if (addr && addr_len) {
			writel(chip, &i2c->iicds);
			/* send START */
			writel(I2C_MODE_MT | I2C_TXRX_ENA | I2C_START_STOP,
			       &i2c->iicstat);
			i = 0;
			while ((i < addr_len) && (result == I2C_OK)) {
				result = WaitForXfer(i2c);
				writel(addr[i], &i2c->iicds);
				ReadWriteByte(i2c);
				i++;
			}
			i = 0;
			while ((i < data_len) && (result == I2C_OK)) {
				result = WaitForXfer(i2c);
				writel(data[i], &i2c->iicds);
				ReadWriteByte(i2c);
				i++;
			}
		} else {
			writel(chip, &i2c->iicds);
			/* send START */
			writel(I2C_MODE_MT | I2C_TXRX_ENA | I2C_START_STOP,
			       &i2c->iicstat);
			i = 0;
			while ((i < data_len) && (result == I2C_OK)) {
				result = WaitForXfer(i2c);
				writel(data[i], &i2c->iicds);
				ReadWriteByte(i2c);
				i++;
			}
		}

		if (result == I2C_OK)
			result = WaitForXfer(i2c);

		/* send STOP */
		writel(I2C_MODE_MT | I2C_TXRX_ENA, &i2c->iicstat);
		ReadWriteByte(i2c);
		break;

	case I2C_READ:
		if (addr && addr_len) {
			writel(chip, &i2c->iicds);
			/* send START */
			writel(I2C_MODE_MT | I2C_TXRX_ENA | I2C_START_STOP,
			       &i2c->iicstat);
			result = WaitForXfer(i2c);
			if (IsACK(i2c)) {
				i = 0;
				while ((i < addr_len) && (result == I2C_OK)) {
					writel(addr[i], &i2c->iicds);
					ReadWriteByte(i2c);
					result = WaitForXfer(i2c);
					i++;
				}

				writel(chip, &i2c->iicds);
				/* resend START */
				writel(I2C_MODE_MR | I2C_TXRX_ENA |
				       I2C_START_STOP, &i2c->iicstat);
				ReadWriteByte(i2c);
				result = WaitForXfer(i2c);
				i = 0;
				while ((i < data_len) && (result == I2C_OK)) {
					/* disable ACK for final READ */
					if (i == data_len - 1)
						writel(readl(&i2c->iiccon)
							& ~I2CCON_ACKGEN,
							&i2c->iiccon);
					ReadWriteByte(i2c);
					result = WaitForXfer(i2c);
					data[i] = readl(&i2c->iicds);
					i++;
				}
			} else {
				result = I2C_NACK;
			}

		} else {
			writel(chip, &i2c->iicds);
			/* send START */
			writel(I2C_MODE_MR | I2C_TXRX_ENA | I2C_START_STOP,
			       &i2c->iicstat);
			result = WaitForXfer(i2c);

			if (IsACK(i2c)) {
				i = 0;
				while ((i < data_len) && (result == I2C_OK)) {
					/* disable ACK for final READ */
					if (i == data_len - 1)
						writel(readl(&i2c->iiccon) &
							~I2CCON_ACKGEN,
							&i2c->iiccon);
					ReadWriteByte(i2c);
					result = WaitForXfer(i2c);
					data[i] = readl(&i2c->iicds);
					i++;
				}
			} else {
				result = I2C_NACK;
			}
		}

		/* send STOP */
		writel(I2C_MODE_MR | I2C_TXRX_ENA, &i2c->iicstat);
		ReadWriteByte(i2c);
		break;

	default:
		debug("i2c_transfer: bad call\n");
		result = I2C_NOK;
		break;
	}

	return (result);
}

int i2c_probe(uchar chip)
{
	struct s3c24x0_i2c *i2c;
	uchar buf[1];

	i2c = get_base_i2c(g_current_bus);
	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	return i2c_transfer(i2c, I2C_READ, chip << 1, 0, 0, buf, 1) != I2C_OK;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	struct s3c24x0_i2c *i2c;
	uchar xaddr[4];
	int ret;

	if (alen > 4) {
		debug("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	i2c = get_base_i2c(g_current_bus);
	ret = i2c_transfer(i2c, I2C_READ, chip << 1, &xaddr[4 - alen], alen,
			buffer, len);
	if (ret) {
		debug("I2c read: failed %d\n", ret);
		return 1;
	}
	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	struct s3c24x0_i2c *i2c;
	uchar xaddr[4];

	if (alen > 4) {
		debug("I2C write: addr len %d not supported\n", alen);
		return 1;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}
#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	i2c = get_base_i2c(g_current_bus);
	return (i2c_transfer
		(i2c, I2C_WRITE, chip << 1, &xaddr[4 - alen], alen, buffer,
		 len) != 0);
}

#endif /* CONFIG_HARD_I2C */
