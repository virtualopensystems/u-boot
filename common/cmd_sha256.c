/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <command.h>
#ifdef CONFIG_EXYNOS_ACE_SHA
#include <asm/arch-exynos5/ace_sha.h>
#else
#include <sha256.h>
#endif

int do_sha256(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long inlen;
	unsigned char *input, *out;
	int i;
#ifndef CONFIG_EXYNOS_ACE_SHA
	sha256_context sha_cnxt;
#endif
	if (argc < 4) {
		printf("usage: sha256 <input> <input length> <output>\n");
		return 0;
	}
	input = (unsigned char *)simple_strtoul(argv[1], NULL, 16);
	inlen = simple_strtoul(argv[2], NULL, 16);
	out = (unsigned char *)simple_strtoul(argv[3], NULL, 16);

#ifdef CONFIG_EXYNOS_ACE_SHA
	ace_sha_hash_digest(out, input, inlen, 2);
#else
	sha256_starts(&sha_cnxt);

	sha256_update(&sha_cnxt, input, inlen);

	sha256_finish(&sha_cnxt, out);
#endif

	for (i = 0; i < 32; i++)
		printf("0x%02X ", out[i]);
	printf("\n");

	return 0;
}

U_BOOT_CMD(
	sha256,	4, 1, do_sha256,
	"print hash result",
	"<input> <inputlength> <output>"
);
