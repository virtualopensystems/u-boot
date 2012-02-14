/*
 * Copyright (C) 2011 Samsung Electronics
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

#include <common.h>
#include <config.h>

/**
 * Copy data from SD or MMC device to RAM.
 *
 * @param offset	Block offset of the data
 * @param nblock	Number of blocks
 * @param dst		Destination address
 * @return 1 = True or 0 = False
 */
typedef u32 (*mmc_copy_fnptr)(u32 offset, u32 nblock, u32 dst);

/**
 * Copy data through USB.
 *
 * @return 1 = True or 0 = False
 */
typedef u32 (*usb_copy_fnptr)(void);

/*
 * Set/clear program flow prediction and return the previous state.
 */
static int config_branch_prediction(int set_cr_z)
{
	unsigned int cr;

	/* System Control Register: 11th bit Z Branch prediction enable */
	cr = get_cr();
	set_cr(set_cr_z ? cr | CR_Z : cr & ~CR_Z);

	return cr & CR_Z;
}

/* Copy U-Boot image to RAM */
static void copy_uboot_to_ram(void)
{
	unsigned int sec_boot_check;
	int is_cr_z_set;
	mmc_copy_fnptr mmc_copy = (void *) *(u32 *)EXYNOS_COPY_MMC_FNPTR_ADDR;
	usb_copy_fnptr usb_copy = (void *) *(u32 *)EXYNOS_COPY_USB_FNPTR_ADDR;

	/* Read iRAM location to check for secondary USB boot mode */
	sec_boot_check = readl(EXYNOS_IRAM_SECONDARY_BASE);
	if (sec_boot_check == EXYNOS_USB_SECONDARY_BOOT) {
		/*
		 * iROM needs program flow prediction to be disabled
		 * before copy from USB device to RAM
		 */
		is_cr_z_set = config_branch_prediction(0);
		usb_copy();
		config_branch_prediction(is_cr_z_set);
		return;
	}

	mmc_copy(BL2_START_OFFSET, BL2_SIZE_BLOC_COUNT, CONFIG_SYS_TEXT_BASE);
}

void board_init_f(unsigned long bootflag)
{
	__attribute__((noreturn)) void (*uboot)(void);

	copy_uboot_to_ram();

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	uboot();
	/* Never returns Here */
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/* Function attribute is no-return */
	/* This Function never executes */
	while (1)
		;
}

void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3) {}
