/*
 * (C) Copyright 2012 Samsung Electronics
 * Register map for Exynos5 PMU
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

#ifndef __EXYNOS5_POWER_H__
#define __EXYNOS5_POWER_H__

#define MIPI_PHY1_CONTROL_ENABLE		(1 << 0)
#define MIPI_PHY1_CONTROL_M_RESETN		(1 << 2)

#define POWER_USB_HOST_PHY_CTRL_EN		(1 << 0)
#define POWER_PS_HOLD_CONTROL_DATA_HIGH		(1 << 8)
#define POWER_ENABLE_HW_TRIP			(1UL << 31)

#define DPTX_PHY_ENABLE		(1 << 0)

/* Power Management Unit register map */
struct exynos5_power {
	/* Add registers as and when required */
	unsigned char	reserved1[0x0400];
	unsigned int	sw_reset;		/* 0x0400 */
	unsigned char	reserved2[0x0304];
	unsigned int	usb_host_phy_ctrl;	/* 0x0708 */
	unsigned char	reserved3[0x8];
	unsigned int	mipi_phy1_control;	/* 0x0714 */
	unsigned char	reserved4[0x8];
	unsigned int	dptx_phy_control;	/* 0x0720 */
	unsigned char	reserved5[0x2be8];
	unsigned int	ps_hold_ctrl;		/* 0x330c */
};

/**
 * Perform a software reset.
 */
void power_reset(void);

/**
 * Power off the system; it should never return.
 */
void power_shutdown(void);

/* Enable DPTX PHY */
void power_enable_dp_phy(void);

void power_enable_usb_phy(void);
void power_disable_usb_phy(void);

/* Enable HW thermal trip with PS_HOLD_CONTROL register ENABLE_HW_TRIP bit */
void power_enable_hw_thermal_trip(void);

/* Initialize the pmic voltages to power up the system */
int power_init(void);

#endif
