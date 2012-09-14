/*
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#include <common.h>

#ifndef CONFIG_SYS_COREBOOT
#error This driver requires coreboot
#endif

#include <asm/arch/sysinfo.h>

struct cbmem_console {
	u32 buffer_size;
	u32 buffer_cursor;
	u8  buffer_body[0];
}  __attribute__ ((__packed__));

static struct cbmem_console *cbmem_console_p;

/*
 * Need to be able to avoid adding charactes to cbmem console buffer while its
 * contents are being dumped.
 */
static int suppress_cbmem_console;

void cbmemc_putc(char data)
{
	int cursor;

	if (suppress_cbmem_console)
		return;

	cursor = cbmem_console_p->buffer_cursor++;
	if (cursor < cbmem_console_p->buffer_size)
		cbmem_console_p->buffer_body[cursor] = data;
}

void cbmemc_puts(const char *str)
{
	char c;

	while ((c = *str++) != 0)
		cbmemc_putc(c);
}

int cbmemc_init(void)
{
	int rc;
	struct stdio_dev cons_dev;
	cbmem_console_p = lib_sysinfo.cbmem_cons;

	memset(&cons_dev, 0, sizeof(cons_dev));

	strcpy(cons_dev.name, "cbmem");
	cons_dev.flags = DEV_FLAGS_OUTPUT; /* Output only */
	cons_dev.putc  = cbmemc_putc;
	cons_dev.puts  = cbmemc_puts;

	rc = stdio_register(&cons_dev);

	return (rc == 0) ? 1 : rc;
}

static int
do_cbc_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int cursor = 0;

	suppress_cbmem_console = 1;

	while (cursor < cbmem_console_p->buffer_cursor)
		putc(cbmem_console_p->buffer_body[cursor++]);

	suppress_cbmem_console = 0;
	return 0;
}

U_BOOT_CMD(cbc_dump, 1, 1, do_cbc_dump,
	   "dump CBMEM console buffer to serial port", NULL);
