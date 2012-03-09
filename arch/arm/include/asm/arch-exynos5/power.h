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

#define POWER_USB_HOST_PHY_CTRL_EN		(1 << 0)
#define POWER_PS_HOLD_CONTROL_DATA_HIGH		(1 << 8)

/* Power Management Unit register map */
struct exynos5_power {
	/* Add registers as and when required */
	unsigned char	reserved1[0x0708];
	unsigned int	usb_host_phy_ctrl;	/* 0x0708 */
	unsigned char	reserved2[0x2c00];
	unsigned int	ps_hold_ctrl;		/* 0x330c */
};

void power_enable_usb_phy(void);
void power_disable_usb_phy(void);

#endif
