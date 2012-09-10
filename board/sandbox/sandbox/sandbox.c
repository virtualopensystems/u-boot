/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
#include <ec_commands.h>
#include <mmc.h>
#include <os.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

void flush_cache(unsigned long start, unsigned long size)
{
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_timer(ulong base)
{
	return (os_get_nsec() / 1000000) - base;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_DRAM_SIZE;
	return 0;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

struct mkbp_dev *board_get_mkbp_dev(void)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return NULL;
}

int mkbp_read_hash(struct mkbp_dev *dev, struct ec_response_vboot_hash *hash)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_flash_update_rw(struct mkbp_dev *dev,
			 const uint8_t  *image, int image_size)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_read_vbnvcontext(struct mkbp_dev *dev, uint8_t *block)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_write_vbnvcontext(struct mkbp_dev *dev, const uint8_t *block)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_flash_protect(struct mkbp_dev *dev,
		       uint32_t set_mask, uint32_t set_flags,
		       struct ec_response_flash_protect *resp)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_reboot(struct mkbp_dev *dev, enum ec_reboot_cmd cmd, uint8_t flags)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int mkbp_read_current_image(struct mkbp_dev *dev, enum ec_current_image *image)
{
#if defined(CONFIG_MKBP)
#error "MKBP configuration not implemented for sandbox"
#endif
	return 0;
}

int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_SANDBOX_MMC
	sandbox_mmc_init(0);
#endif
	return 0;
}
#endif
