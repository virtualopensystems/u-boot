/*
 * SAMSUNG S5P USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <fdtdec.h>
#include <usb.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/power.h>
#include <asm/arch/ehci-s5p.h>
#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

/* reset line to the HSIC USB chip */
static struct fdt_gpio_state hsichub_reset;

int __board_usb_vbus_init(void)
{
	/* placeholder for board specific VBUS initialization */
	return 0;
}

int board_usb_vbus_init(void)
	__attribute__((weak, alias("__board_usb_vbus_init")));

/* Setup the EHCI host controller. */
static void setup_usb_phy(struct usb_phy *usb)
{
	unsigned int hostphy_ctrl0;
	int node;

	/* Enable VBUS */
	board_usb_vbus_init();

	power_enable_usb_phy();

	/* Setting up host and device simultaneously */
	hostphy_ctrl0 = readl(&usb->usbphyctrl0);
	hostphy_ctrl0 &= ~(HOST_CTRL0_FSEL_MASK |
			   HOST_CTRL0_COMMONON_N |
			   /* HOST Phy setting */
			   HOST_CTRL0_PHYSWRST |
			   HOST_CTRL0_PHYSWRSTALL |
			   HOST_CTRL0_SIDDQ |
			   HOST_CTRL0_FORCESUSPEND |
			   HOST_CTRL0_FORCESLEEP);
	hostphy_ctrl0 |= (/* Setting up the ref freq */
			  CLK_24MHZ << 16 |
			  /* HOST Phy setting */
			  HOST_CTRL0_LINKSWRST |
			  HOST_CTRL0_UTMISWRST);
	writel(hostphy_ctrl0, &usb->usbphyctrl0);
	udelay(10);
	clrbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_LINKSWRST |
			HOST_CTRL0_UTMISWRST);
	udelay(20);

	/* EHCI Ctrl setting */
	setbits_le32(&usb->ehcictrl,
			EHCICTRL_ENAINCRXALIGN |
			EHCICTRL_ENAINCR4 |
			EHCICTRL_ENAINCR8 |
			EHCICTRL_ENAINCR16);

	/* HSIC USB Hub initialization. */
	node = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_SMSC_USB3503);
	if (node > 0) {
		fdtdec_decode_gpio(gd->fdt_blob, node, "reset-gpio",
			&hsichub_reset);
		fdtdec_setup_gpio(&hsichub_reset);
		gpio_direction_output(hsichub_reset.gpio, 0);
		udelay(100);
		gpio_direction_output(hsichub_reset.gpio, 1);
		udelay(5000);

		clrbits_le32(&usb->hsicphyctrl1,
				HOST_CTRL0_SIDDQ |
				HOST_CTRL0_FORCESLEEP |
				HOST_CTRL0_FORCESUSPEND);
		setbits_le32(&usb->hsicphyctrl1, HOST_CTRL0_PHYSWRST);
		udelay(10);
		clrbits_le32(&usb->hsicphyctrl1, HOST_CTRL0_PHYSWRST);
	}

	/* PHY clock and power setup time */
	mdelay(50);
}

/* Reset the EHCI host controller. */
static void reset_usb_phy(struct usb_phy *usb)
{
	/* HOST_PHY reset */
	setbits_le32(&usb->usbphyctrl0,
			HOST_CTRL0_PHYSWRST |
			HOST_CTRL0_PHYSWRSTALL |
			HOST_CTRL0_SIDDQ |
			HOST_CTRL0_FORCESUSPEND |
			HOST_CTRL0_FORCESLEEP);

	/* reset HSIC PHY and remote USB chip if we have one */
	if (hsichub_reset.gpio) {
		setbits_le32(&usb->hsicphyctrl1, HOST_CTRL0_SIDDQ |
						 HOST_CTRL0_FORCESLEEP |
						 HOST_CTRL0_FORCESUSPEND |
						 HOST_CTRL0_PHYSWRST);

		gpio_direction_output(hsichub_reset.gpio, 0);
	}

	power_disable_usb_phy();
}

/*
 * EHCI-initialization
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, struct ehci_hccr **ret_hccr,
				struct ehci_hcor **ret_hcor)
{
	struct usb_phy *usb;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
				      COMPAT_SAMSUNG_EXYNOS_EHCI);
	if (node < 0)
		return node;

	hccr = (struct ehci_hccr *)fdtdec_get_addr(gd->fdt_blob, node, "reg");
	if (!hccr) {
		debug("%s: no registers address for EHCI\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}

	ehci_hcd_set_port_enable_mask(index, fdtdec_get_int(gd->fdt_blob, node,
						"port-enable-mask", -1U));

	usb = (struct usb_phy *)samsung_get_base_usb_phy();
	setup_usb_phy(usb);

	hcor = (struct ehci_hcor *)((uint32_t) hccr
				+ HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	debug("Exynos5-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)hccr, (uint32_t)hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	*ret_hccr = hccr;
	*ret_hcor = hcor;
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	struct usb_phy *usb;

	usb = (struct usb_phy *)samsung_get_base_usb_phy();
	reset_usb_phy(usb);

	return 0;
}
