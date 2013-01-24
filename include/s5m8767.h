/*
 *  Copyright (C) 2013 Google Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __S5M8767_H_
#define __S5M8767_H_

enum s5m8767_regnum {
	S5M8767_BUCK1 = 0,
	S5M8767_BUCK2,
	S5M8767_BUCK3,
	S5M8767_BUCK4,
	S5M8767_BUCK5,
	S5M8767_BUCK6,
	S5M8767_BUCK7,
	S5M8767_BUCK8,
	S5M8767_BUCK9,
	S5M8767_LDO1,
	S5M8767_LDO2,
	S5M8767_LDO3,
	S5M8767_LDO4,
	S5M8767_LDO5,
	S5M8767_LDO6,
	S5M8767_LDO7,
	S5M8767_LDO8,
	S5M8767_LDO9,
	S5M8767_LDO10,
	S5M8767_LDO11,
	S5M8767_LDO12,
	S5M8767_LDO13,
	S5M8767_LDO14,
	S5M8767_LDO15,
	S5M8767_LDO16,
	S5M8767_LDO17,
	S5M8767_LDO18,
	S5M8767_LDO19,
	S5M8767_LDO20,
	S5M8767_LDO21,
	S5M8767_LDO22,
	S5M8767_LDO23,
	S5M8767_LDO24,
	S5M8767_LDO25,
	S5M8767_LDO26,
	S5M8767_LDO27,
	S5M8767_LDO28,
	S5M8767_EN32KHZ_CP,
};

/**
 * struct s5m8767_para - s5m8767 register parameters
 * @param vol_addr	i2c address of the given buck/ldo register
 * @param vol_bitpos	bit position to be set or clear within register
 * @param vol_bitmask	bit mask value
 * @param reg_enaddr	control register address, which enable the given
 *			given buck/ldo.
 * @param reg_enbitpos	bit position to be enabled
 * @param reg_enbiton	value to be written to buck/ldo to make it ON
 * @param reg_enbitoff	value to be written to buck/ldo to make it OFF
 * @param vol_min	minimum voltage level supported by given buck/ldo
 * @param vol_div	voltage division value of given buck/ldo
 */
struct s5m8767_para {
	enum s5m8767_regnum regnum;
	u8	vol_addr;
	u8	vol_bitpos;
	u8	vol_bitmask;
	u8	reg_enaddr;
	u8	reg_enbitpos;
	u8	reg_enbitmask;
	u8	reg_enbiton;
	u8	reg_enbitoff;
	u32	vol_min;
	u32	vol_div;
};

/* I2C device address for pmic s5m8767 */
#define S5M8767_I2C_ADDR 0x66

enum {
	S5M8767_REG_DISABLE = 0,
	S5M8767_REG_ENABLE
};

enum {
	S5M8767_MV = 0,	/* mili volt */
	S5M8767_UV		/* micro volt */
};

/**
 * This function enables the 32KHz coprocessor clock.
 *
 * Return 0 if ok, else -1
 */
int s5m8767_enable_32khz_cp(void);

/**
 * Set the required voltage level of pmic
 *
 * @param reg		register number of buck/ldo to be set
 * @param volt		voltage level to be set
 * @param enable	enable or disable bit
 * @param volt_units	S5M8767_MV or S5M8767_UV, unit of the
 *			voltage parameters
 *
 * @return		Return 0 if ok, else -1
 */
int s5m8767_volsetting(enum s5m8767_regnum reg, unsigned int volt,
			int enable, int volt_units);

/**
 * Disable charging of the RTC backup battery
 *
 * @return		Return 0 if ok, else -1
 */
int s5m8767_disable_backup_batt(void);

#endif /* __S5M8767_PMIC_H_ */
