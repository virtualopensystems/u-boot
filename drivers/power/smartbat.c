/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * Smart battery driver
 */

#include <battery.h>
#include <common.h>
#include <i2c.h>
#include <smartbat.h>

/* Registers we can read/write */
enum {
	REG_TEMPERATURE = 8,
	REG_VOLTAGE,
	REG_AVERAGE_CURRENT = 0xb,
	REG_RELATIVE_CHARGE = 0xd,
	REG_BATTERY_STATUS = 0x16,
	REG_DESIGN_CAP = 0x18,
	REG_DESIGN_VOLTAGE,
	REG_SPEC_INFO,
	REG_MANUF_DATE,
	REG_SERIAL_NUM,
	REG_MANUF_NAME = 0x20,
	REG_DEVICE_NAME,
	REG_DEVICE_CHEM,
};

static struct {
	int bus;
	int addr;
	int old_bus;
} config;

#if 0  /* Currently not needed */
/**
 * Write a value to a register
 *
 * @param	reg_addr	register address to write
 * @param	value		value to be written
 * @return	0 on success, non-0 on failure
 */
static int smartbat_i2c_write(unsigned int reg_addr, unsigned value)
{
	uint8_t buff[2];

	buff[0] = value & 0xff;
	buff[1] = (value >> 8) & 0xff;
	return i2c_write(config.addr, reg_addr, 1, buff, 2);
}
#endif

/**
 * Read a halfword from the smartbat
 *
 * @param	reg_addr	register address to read
 * @param	value		address to store the value to be read
 * @return	0 on success, non-0 on failure
 */
static int smartbat_i2c_read(unsigned int reg_addr, unsigned *value)
{
	uint8_t buff[2];

	if (i2c_read(config.addr, reg_addr, 1, buff, 2)) {
		debug("%s: Read failure, reg %#x\n", __func__, reg_addr);
		return -1;
	}
	*value = buff[0] + (buff[1] << 8);

	return 0;
}

/**
 * Read a string from the smartbat
 *
 * The string starts with a length byte, so we read that first.
 *
 * @param	reg_addr	register address to read
 * @param	str		Place to put string that is read.
 * @param	maxlen		Maximum length of string to read, should be
 *				at least 2 bytes (1 for the length, another
 *				for the \0 terminator)
 * @return	0 on success, non-0 on failure
 */
static int smartbat_i2c_read_string(unsigned int reg_addr, char *str,
				  int maxlen)
{
	uint8_t len;

	assert(maxlen >= 2);
	if (i2c_read(config.addr, reg_addr, 1, &len, 1)) {
		debug("%s: Read failure, addr %u, reg %#x\n", __func__,
		      config.addr, reg_addr);
		return -1;
	}
	len = min(len, maxlen - 2);
	if (i2c_read(config.addr, reg_addr, 1, (uint8_t *)str, len + 1)) {
		debug("%s: Read failure, len %d\n", __func__, len);
		return -1;
	}
	memmove(str, str + 1, len);
	str[len] = '\0';

	return 0;
}

static int smartbat_select(void)
{
	int ret;

	config.old_bus = i2c_get_bus_num();
	if (config.old_bus != config.bus) {
		debug("%s: Select bus %d\n", __func__, config.bus);
		ret = i2c_set_bus_num(config.bus);
		if (ret) {
			debug("%s: Cannot select battery, err %d\n",
			      __func__, ret);
			return -1;
		}
	}

	return 0;
}

static int smartbat_deselect(void)
{
	int ret;

	if (config.old_bus != i2c_get_bus_num()) {
		ret = i2c_set_bus_num(config.old_bus);
		debug("%s: Select bus %d\n", __func__, config.old_bus);
		if (ret) {
			debug("%s: Cannot restore i2c bus, err %d\n",
			      __func__, ret);
			return -1;
		}
	}
	config.old_bus = -1;
	return 0;
}

int smartbat_get_temperature(int *milli_degrees_c)
{
	unsigned temp;
	int ret;

	if (smartbat_select())
		return -1;
	ret = smartbat_i2c_read(REG_TEMPERATURE, &temp);
	smartbat_deselect();
	if (ret)
		return -1;

	/* Convert to milli-degress celsius */
	*milli_degrees_c = (temp * 100) - 273150;

	return 0;
}

int smartbat_get_info(struct battery_info *info)
{
	unsigned value;
	int err;

	if (smartbat_select())
		return -1;
	err = smartbat_i2c_read_string(REG_MANUF_NAME, info->manuf_name,
				    sizeof(info->manuf_name));
	err |= smartbat_i2c_read_string(REG_DEVICE_NAME, info->device_name,
				    sizeof(info->manuf_name));
	err |= smartbat_i2c_read_string(REG_DEVICE_CHEM, info->device_chem,
				    sizeof(info->manuf_name));
	err |= smartbat_i2c_read(REG_SERIAL_NUM, &info->serial_num);
	err |= smartbat_i2c_read(REG_DESIGN_CAP, &info->design_cap_mah);
	err |= smartbat_i2c_read(REG_DESIGN_VOLTAGE, &info->design_voltage_mv);
	err |= smartbat_i2c_read(REG_VOLTAGE, &info->voltage_mv);
	err |= smartbat_i2c_read(REG_AVERAGE_CURRENT, &value);
	info->average_current_ma = (short)value;
	err |= smartbat_i2c_read(REG_RELATIVE_CHARGE, &info->relative_charge);
	err |= smartbat_i2c_read(REG_BATTERY_STATUS, &info->battery_status);
	smartbat_deselect();

	return err;
}

int smartbat_get_status(void)
{
	unsigned int status;
	int err;

	if (smartbat_select())
		return -1;
	err = smartbat_i2c_read(REG_BATTERY_STATUS, &status);
	smartbat_deselect();

	return err ? -1 : status;
}

int smartbat_init(void)
{
	int ret;

	/* TODO(sjg): Move to fdt */
	config.old_bus = -1;
	config.bus = 4;
	config.addr = 0x0b;

	if (smartbat_select())
		return -1;

	/* Probe the chip */
	ret = i2c_probe(config.addr);
	smartbat_deselect();

	return ret;
}
