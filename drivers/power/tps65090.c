/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <i2c.h>
#include <tps65090.h>

/**
 * Write a value to a register
 *
 * @param	chip_addr	i2c slave addr for max77686
 * @param	reg_addr	register address to write
 * @param	value		value to be written
 * @return	0 on success, non-0 on failure
 */
static inline int tps65090_i2c_write(unsigned char chip_addr,
				     unsigned int reg_addr, unsigned char value)
{
	return i2c_write(chip_addr, reg_addr, 1, &value, 1);
}

/**
 * Read a value from a register
 *
 * @param	chip_addr	i2c addr for max77686
 * @param	reg_addr	register address to read
 * @param	value		address to store the value to be read
 * @return	0 on success, non-0 on failure
 */
static inline int tps65090_i2c_read(unsigned char chip_addr,
				    unsigned int reg_addr, unsigned char *value)
{
	return i2c_read(chip_addr, reg_addr, 1, value, 1);
}

int tps65090_fet_enable(unsigned int fet_id)
{
	unsigned char reg;
	int ret;

	if (fet_id == 0 || fet_id > TPS65090_MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
				fet_id, TPS65090_MAX_FET_NUM);
		return -1;
	}

	ret = tps65090_i2c_write(TPS65090_I2C_ADDR,
			TPS65090_REG_FET1_CTRL + fet_id - 1,
			TPS65090_FET_CTRL_ADENFET | TPS65090_FET_CTRL_ENFET);
	if (ret)
		return ret;

	ret = tps65090_i2c_read(TPS65090_I2C_ADDR,
			TPS65090_REG_FET1_CTRL + fet_id - 1,
			&reg);
	if (ret)
		return ret;

	if (!(reg & TPS65090_FET_CTRL_PGFET)) {
		debug("still no power after enable FET%d\n", fet_id);
		return -2;
	}

	return ret;
}

int tps65090_fet_disable(unsigned int fet_id)
{
	unsigned char reg;
	int ret;

	if (fet_id == 0 || fet_id > TPS65090_MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
				fet_id, TPS65090_MAX_FET_NUM);
		return -1;
	}

	ret = tps65090_i2c_write(TPS65090_I2C_ADDR,
			TPS65090_REG_FET1_CTRL + fet_id - 1,
			TPS65090_FET_CTRL_ADENFET);
	if (ret)
		return ret;

	ret = tps65090_i2c_read(TPS65090_I2C_ADDR,
			TPS65090_REG_FET1_CTRL + fet_id - 1,
			&reg);
	if (ret)
		return ret;

	if (reg & TPS65090_FET_CTRL_PGFET) {
		debug("still power good after disable FET%d\n", fet_id);
		return -2;
	}

	return ret;
}

int tps65090_fet_is_enabled(unsigned int fet_id)
{
	unsigned char reg;
	int ret;

	if (fet_id == 0 || fet_id > TPS65090_MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
				fet_id, TPS65090_MAX_FET_NUM);
		return -1;
	}

	ret = tps65090_i2c_read(TPS65090_I2C_ADDR,
			TPS65090_REG_FET1_CTRL + fet_id - 1,
			&reg);
	if (ret) {
		debug("fail to read FET%u_CTRL register over I2C", fet_id);
		return -2;
	}

	return reg & TPS65090_FET_CTRL_ENFET;
}

int tps65090_init(void)
{
	int ret;

	/* Init the I2C so that we can program TPS65090 chip */
	ret = i2c_set_bus_num(CONFIG_TPS65090_I2C_BUS);
	if (ret) {
		debug("failed to set TPS65090 I2C bus 0x%x, returned %d\n",
		      CONFIG_TPS65090_I2C_BUS, ret);
		return ret;
	}

	/* Probe the chip */
	ret = i2c_probe(TPS65090_I2C_ADDR);
	if (ret) {
		debug("failed to probe TPS65090 over I2C, returned %d\n", ret);
		return ret;
	}

	return 0;
}
