/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <i2c.h>
#include <s5m8767.h>

/* Chip register numbers (not exported from this module) */
enum {
	REG_BBAT		= 0x0e,

	/* Bits for BBAT */
	BBAT_BBCHOSTEN_MASK	= 1 << 0,
	BBAT_BBCVS_SHIFT	= 3,
	BBAT_BBCVS_MASK		= 3 << BBAT_BBCVS_SHIFT,
};

struct s5m8767_para s5m8767_param[] = {/*{regnum, vol_addr, vol_bitpos,
	vol_bitmask, reg_enaddr, reg_enbitpos, reg_enbitmask, reg_enbiton,
	reg_enbitoff, vol_min, vol_div}*/
	/*           | voltage ----|  | enable --------------|  voltage  */
	/*regnum     addr  bpos mask  addr  bpos mask on   off  min  div */
	{S5M8767_BUCK1, 0x33, 0x0, 0xFF, 0x32, 0x6, 0x3, 0x3, 0x0, 650, 6250},
	{S5M8767_BUCK2, 0x35, 0x0, 0xFF, 0x34, 0x6, 0x3, 0x1, 0x0, 600, 6250},
	{S5M8767_BUCK3, 0x3E, 0x0, 0xFF, 0x3D, 0x6, 0x3, 0x1, 0x0, 600, 6250},
	{S5M8767_BUCK4, 0x47, 0x0, 0xFF, 0x46, 0x6, 0x3, 0x1, 0x0, 600, 6250},
	{S5M8767_BUCK5, 0x50, 0x0, 0xFF, 0x4F, 0x6, 0x3, 0x3, 0x0, 650, 6250},
	{S5M8767_BUCK6, 0x55, 0x0, 0xFF, 0x54, 0x6, 0x3, 0x3, 0x0, 650, 6250},
	{S5M8767_BUCK7, 0x57, 0x0, 0xFF, 0x56, 0x6, 0x3, 0x3, 0x0, 750, 12500},
	{S5M8767_BUCK8, 0x59, 0x0, 0xFF, 0x58, 0x6, 0x3, 0x3, 0x0, 750, 12500},
	{S5M8767_BUCK9, 0x5B, 0x0, 0xFF, 0x5A, 0x6, 0x3, 0x3, 0x0, 750, 12500},
	{S5M8767_LDO1,  0x5C, 0x0, 0x3F, 0x5C, 0x6, 0x3, 0x3, 0x0, 800, 25000},
	{S5M8767_LDO2,  0x5D, 0x0, 0x3F, 0x5D, 0x6, 0x3, 0x1, 0x0, 800, 25000},
	{S5M8767_LDO3,  0x61, 0x0, 0x3F, 0x61, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO4,  0x62, 0x0, 0x3F, 0x62, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO5,  0x63, 0x0, 0x3F, 0x63, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO6,  0x64, 0x0, 0x3F, 0x64, 0x6, 0x3, 0x1, 0x0, 800, 25000},
	{S5M8767_LDO7,  0x65, 0x0, 0x3F, 0x65, 0x6, 0x3, 0x1, 0x0, 800, 25000},
	{S5M8767_LDO8,  0x66, 0x0, 0x3F, 0x66, 0x6, 0x3, 0x1, 0x0, 800, 25000},
	{S5M8767_LDO9,  0x67, 0x0, 0x3F, 0x67, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO10, 0x68, 0x0, 0x3F, 0x68, 0x6, 0x3, 0x1, 0x0, 800, 50000},
	{S5M8767_LDO11, 0x69, 0x0, 0x3F, 0x69, 0x6, 0x3, 0x1, 0x0, 800, 50000},
	{S5M8767_LDO12, 0x6A, 0x0, 0x3F, 0x6A, 0x6, 0x3, 0x1, 0x0, 800, 50000},
	{S5M8767_LDO13, 0x6B, 0x0, 0x3F, 0x6B, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO14, 0x6C, 0x0, 0x3F, 0x6C, 0x6, 0x3, 0x1, 0x0, 800, 50000},
	{S5M8767_LDO15, 0x6D, 0x0, 0x3F, 0x6D, 0x6, 0x3, 0x1, 0x0, 800, 25000},
	{S5M8767_LDO16, 0x6E, 0x0, 0x3F, 0x6E, 0x6, 0x3, 0x1, 0x0, 800, 50000},
	{S5M8767_LDO17, 0x6F, 0x0, 0x3F, 0x6F, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO18, 0x70, 0x0, 0x3F, 0x70, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO19, 0x71, 0x0, 0x3F, 0x71, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO20, 0x72, 0x0, 0x3F, 0x72, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO21, 0x73, 0x0, 0x3F, 0x73, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO22, 0x74, 0x0, 0x3F, 0x74, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO23, 0x75, 0x0, 0x3F, 0x75, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO24, 0x76, 0x0, 0x3F, 0x76, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO25, 0x77, 0x0, 0x3F, 0x77, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO26, 0x78, 0x0, 0x3F, 0x78, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO27, 0x79, 0x0, 0x3F, 0x79, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_LDO28, 0x7A, 0x0, 0x3F, 0x7A, 0x6, 0x3, 0x3, 0x0, 800, 50000},
	{S5M8767_EN32KHZ_CP, 0x0, 0x0, 0x0, 0x0A, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0},
};

/*
 * Write a value to a register
 *
 * @param chip_addr	i2c addr for s5m8767
 * @param reg		reg number to write
 * @param val		value to be written
 *
 */
static inline int s5m8767_i2c_write(unsigned char chip_addr,
					unsigned int reg, unsigned char val)
{
	return i2c_write(chip_addr, reg, 1, &val, 1);
}

/*
 * Read a value from a register
 *
 * @param chip_addr	i2c addr for s5m8767
 * @param reg		reg number to write
 * @param val		value to be written
 *
 */
static inline int s5m8767_i2c_read(unsigned char chip_addr,
					unsigned int reg, unsigned char *val)
{
	return i2c_read(chip_addr, reg, 1, val, 1);
}

/*
 * Enable the s5m8767 register
 *
 * @param reg		register number of buck/ldo to be enabled
 * @param enable	enable or disable bit
 *
 *			S5M8767_REG_DISABLE = 0,
			needed to set the buck/ldo enable bit OFF
 * @return		Return 0 if ok, else -1
 */
static int s5m8767_enablereg(enum s5m8767_regnum reg, int enable)
{
	struct s5m8767_para *pmic;
	unsigned char read_data;
	int ret;

	pmic = &s5m8767_param[reg];

	ret = s5m8767_i2c_read(S5M8767_I2C_ADDR, pmic->reg_enaddr,
				&read_data);
	if (ret != 0) {
		debug("s5m8767 i2c read failed.\n");
		return -1;
	}

	if (enable == S5M8767_REG_DISABLE) {
		clrbits_8(&read_data,
				pmic->reg_enbitmask << pmic->reg_enbitpos);
	} else {
		clrsetbits_8(&read_data,
				pmic->reg_enbitmask << pmic->reg_enbitpos,
				pmic->reg_enbiton << pmic->reg_enbitpos);
	}

	ret = s5m8767_i2c_write(S5M8767_I2C_ADDR,
				 pmic->reg_enaddr, read_data);
	if (ret != 0) {
		debug("s5m8767 i2c write failed.\n");
		return -1;
	}

	return 0;
}

static int s5m8767_do_volsetting(enum s5m8767_regnum reg, unsigned int volt,
				  int enable, int volt_units)
{
	struct s5m8767_para *pmic;
	unsigned char read_data;
	int vol_level = 0;
	int ret;

	pmic = &s5m8767_param[reg];

	if (pmic->vol_addr == 0) {
		debug("not a voltage register.\n");
		return -1;
	}

	ret = s5m8767_i2c_read(S5M8767_I2C_ADDR, pmic->vol_addr, &read_data);
	if (ret != 0) {
		debug("s5m8767 i2c read failed.\n");
		return -1;
	}

	if (volt_units == S5M8767_UV)
		vol_level = volt - pmic->vol_min * 1000;
	else
		vol_level = (volt - pmic->vol_min) * 1000;

	if (vol_level < 0) {
		debug("Not a valid voltage level to set\n");
		return -1;
	}
	vol_level /= pmic->vol_div;

	clrsetbits_8(&read_data, pmic->vol_bitmask << pmic->vol_bitpos,
			vol_level << pmic->vol_bitpos);

	ret = s5m8767_i2c_write(S5M8767_I2C_ADDR, pmic->vol_addr, read_data);
	if (ret != 0) {
		debug("s5m8767 i2c write failed.\n");
		return -1;
	}

	ret = s5m8767_enablereg(reg, enable);
	if (ret != 0) {
		debug("Failed to enable buck/ldo.\n");
		return -1;
	}

	return 0;
}

int s5m8767_volsetting(enum s5m8767_regnum reg, unsigned int volt,
			int enable, int volt_units)
{
	int old_bus = i2c_get_bus_num();
	int ret;

	i2c_set_bus_num(0);
	ret = s5m8767_do_volsetting(reg, volt, enable, volt_units);
	i2c_set_bus_num(old_bus);
	return ret;
}

int s5m8767_enable_32khz_cp(void)
{
	i2c_set_bus_num(0);
	return s5m8767_enablereg(S5M8767_EN32KHZ_CP, S5M8767_REG_ENABLE);
}

int s5m8767_disable_backup_batt(void)
{
	unsigned char val;
	int ret;

	i2c_set_bus_num(0);
	ret = s5m8767_i2c_read(S5M8767_I2C_ADDR, REG_BBAT, &val);
	if (ret) {
		debug("s5m8767 i2c read failed\n");
		return ret;
	}

	/* If we already have the correct values, exit */
	if ((val & (BBAT_BBCVS_MASK | BBAT_BBCHOSTEN_MASK)) ==
			BBAT_BBCVS_MASK)
		return 0;

	/* First disable charging */
	val &= ~BBAT_BBCHOSTEN_MASK;
	ret = s5m8767_i2c_write(S5M8767_I2C_ADDR, REG_BBAT, val);
	if (ret) {
		debug("s5m8767 i2c write failed\n");
		return -1;
	}

	/* Finally select 3.5V to minimize power consumption */
	val |= BBAT_BBCVS_MASK;
	ret = s5m8767_i2c_write(S5M8767_I2C_ADDR, REG_BBAT, val);
	if (ret) {
		debug("s5m8767 i2c write failed\n");
		return -1;
	}

	return 0;
}
