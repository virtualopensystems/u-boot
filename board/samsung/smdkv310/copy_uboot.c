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

#include<common.h>
#include<config.h>

/*
* Copy U-boot from mmc to RAM:
* COPY_BL2_FNPTR_ADDR: Address in iRAM, which Contains
* API (Data transfer from mmc to ram)
*/
void copy_uboot_to_ram(void)
{
	u32 (*copy_bl2)(u32, u32, u32) = (void *)COPY_BL2_FNPTR_ADDR;

	copy_bl2(BL2_START_OFFSET, BL2_SIZE_BLOC_COUNT, CONFIG_SYS_TEXT_BASE);
}