/*
 * Copyright (C) 2012 Samsung Electronics
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

#ifndef _S3C24X0_I2C_H
#define _S3C24X0_I2C_H

/* I2C channels exynos5 has 8 i2c channel */
#define I2C0		0
#define I2C1		1
#define I2C2		2
#define I2C3		3
#define I2C4		4
#define I2C5		5
#define I2C6		6
#define I2C7		7

#ifdef CONFIG_EXYNOS5
#define CONFIG_MAX_I2C_NUM	8
#else
#define CONFIG_MAX_I2C_NUM	1
#endif

struct s3c24x0_i2c {
	u32	iiccon;
	u32	iicstat;
	u32	iicadd;
	u32	iicds;
	u32	iiclc;
	uchar	res1[0xffec];
};
#endif /* _S3C24X0_I2C_H */
