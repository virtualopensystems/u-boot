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

/*
 * This is a serial port provided by the flattened device tree. It works
 * by selecting a compiled in driver according to the setting in the FDT.
 */

#include <common.h>
#include <fdtdec.h>
#include <ns16550.h>
#include <serial.h>
#include <linux/compiler.h>


DECLARE_GLOBAL_DATA_PTR;

/* Information obtained about a UART from the FDT */
struct fdt_uart {
	fdt_addr_t reg;	/* address of registers in physical memory */
	int id;		/* id or port number (numbered from 0, default -1) */
	int reg_shift;	/* each register is (1 << reg_shift) apart */
	int baudrate;	/* baud rate, will be gd->baudrate if not defined */
	int clock_freq;	/* clock frequency, -1 if not provided */
	int multiplier;	/* divisor multiplier, default 16 */
	int divisor;	/* baud rate divisor, default calculated */
	int enabled;	/* 1 to enable, 0 to disable */
	int interrupt;	/* interrupt line */
	int silent;	/* 1 for silent UART (supresses output by default) */
	int io_mapped;	/* 1 for IO mapped UART, 0 for memory mapped UART */
	enum fdt_compat_id compat; /* our selected driver */
};

/*
 * We need these structures to be outside BSS since they are accessed before
 * relocation.
 */
static struct serial_device console = {
	"not_in_bss"
};

struct fdt_uart console_uart = {
	-1U
};

static void uart_calc_divisor(struct fdt_uart *uart)
{
	if (uart->multiplier && uart->baudrate)
		uart->divisor = (uart->clock_freq +
				(uart->baudrate * (uart->multiplier / 2))) /
			(uart->multiplier * uart->baudrate);
}

static int decode_uart_console(const void *blob, struct fdt_uart *uart,
		int default_baudrate)
{
	int node;

	memset(uart, '\0', sizeof(*uart));
	uart->compat = COMPAT_UNKNOWN;
	node = fdtdec_find_alias_node(blob, "console");
	if (node < 0) {
		debug("%s: Cannot find console alias in fdt\n", __func__);
		return 0;
	}
	uart->reg = fdtdec_get_addr(blob, node, "reg");
	uart->id = 3;
	uart->reg_shift = fdtdec_get_int(blob, node, "reg-shift", 2);
	uart->baudrate = fdtdec_get_int(blob, node, "baudrate",
					default_baudrate);
	uart->clock_freq = fdtdec_get_int(blob, node, "clock-frequency", -1);
	uart->multiplier = fdtdec_get_int(blob, node, "multiplier", 16);
	uart->divisor = fdtdec_get_int(blob, node, "divisor", -1);
	uart->enabled = fdtdec_get_is_enabled(blob, node);
	uart->interrupt = fdtdec_get_int(blob, node, "interrupts", -1);
	uart->silent = fdtdec_get_config_int(blob, "silent_console", 0);
	uart->io_mapped = fdtdec_get_int(blob, node, "io-mapped", 0);
	uart->compat = fdtdec_lookup(blob, node);

	/* Calculate divisor if required */
	if ((uart->divisor == -1) && (uart->clock_freq != -1))
		uart_calc_divisor(uart);
	return 0;
}

/* Access the console - this may need to be a function */
#define DECLARE_CONSOLE	struct fdt_uart *uart = &console_uart

/* Initialize the serial port */
static int fserial_init(void)
{
	DECLARE_CONSOLE;

	switch (uart->compat) {
#ifdef CONFIG_SYS_NS16550
	case COMPAT_SERIAL_TEGRA20_UART:
	case COMPAT_SERIAL_NS16550:
#ifdef CONFIG_SYS_NS16550_RUNTIME_MAPPED
		NS16550_is_io_mapped(uart->io_mapped);
#endif
		NS16550_init((NS16550_t)uart->reg, uart->divisor);
		break;
#endif
	default:
		break;
	}
#ifdef CONFIG_SILENT_CONSOLE
	/* if the console UART wants to be silent, do this now */
	if (uart->silent)
		gd->flags |= GD_FLG_SILENT;
#endif
	return 0;
}

static void fserial_putc(const char c)
{
	DECLARE_CONSOLE;

	if (c == '\n')
		fserial_putc('\r');
	switch (uart->compat) {
#ifdef CONFIG_SYS_NS16550
	case COMPAT_SERIAL_TEGRA20_UART:
	case COMPAT_SERIAL_NS16550:
#ifdef CONFIG_SYS_NS16550_RUNTIME_MAPPED
		NS16550_is_io_mapped(uart->io_mapped);
#endif
		NS16550_putc((NS16550_t)uart->reg, c);
		break;
#endif
	default:
		break;
	}
}

static void fserial_puts(const char *s)
{
	while (*s)
		fserial_putc(*s++);
}

static int fserial_getc(void)
{
	DECLARE_CONSOLE;

	switch (uart->compat) {
	case COMPAT_UNKNOWN:
		return 0;
#ifdef CONFIG_SYS_NS16550
	case COMPAT_SERIAL_TEGRA20_UART:
	case COMPAT_SERIAL_NS16550:
#ifdef CONFIG_SYS_NS16550_RUNTIME_MAPPED
		NS16550_is_io_mapped(uart->io_mapped);
#endif
		return NS16550_getc((NS16550_t)uart->reg);
#endif
	default:
		break;
	}
	hang();

	return 0;
}

static int fserial_tstc(void)
{
	DECLARE_CONSOLE;

	switch (uart->compat) {
#ifdef CONFIG_SYS_NS16550
	case COMPAT_SERIAL_TEGRA20_UART:
	case COMPAT_SERIAL_NS16550:
#ifdef CONFIG_SYS_NS16550_RUNTIME_MAPPED
		NS16550_is_io_mapped(uart->io_mapped);
#endif
		return NS16550_tstc((NS16550_t)uart->reg);
#endif
	default:
		break;
	}
	return 0;
}

static void fserial_setbrg(void)
{
	DECLARE_CONSOLE;

	uart->baudrate = gd->baudrate;
	uart_calc_divisor(uart);
	switch (uart->compat) {
#ifdef CONFIG_SYS_NS16550
	case COMPAT_SERIAL_TEGRA20_UART:
	case COMPAT_SERIAL_NS16550:
#ifdef CONFIG_SYS_NS16550_RUNTIME_MAPPED
		NS16550_is_io_mapped(uart->io_mapped);
#endif
		NS16550_reinit((NS16550_t)uart->reg, uart->divisor);
		break;
#endif
	default:
		break;
	}
}

static struct serial_device *get_console(void)
{
	if (decode_uart_console(gd->fdt_blob, &console_uart,
			gd->baudrate))
		return NULL;
	if (!console_uart.enabled)
		return NULL;
	strcpy(console.name, "serial");
	console.init = fserial_init;
	console.uninit = NULL;
	console.setbrg = fserial_setbrg;
	console.getc = fserial_getc;
	console.tstc = fserial_tstc;
	console.putc = fserial_putc;
	console.puts = fserial_puts;
	return &console;
}

struct serial_device *serial_fdt_get_console_r(void)
{
	/*
	 * Relocation moves all our function pointers, so we need to set up
	 * things again. This function will only be called once.
	 *
	 * We cannot do the -1 check as in default_serial_console()
	 * because it will be -1 if that function has been ever been called.
	 * However, the function pointers set up by serial_fdt_get_console_f
	 * will be pre-relocation values, so we must re-calculate them.
	 */
	return get_console();
}

__weak struct serial_device *default_serial_console(void)
{
	/* if the uart isn't already set up, do it now */
	if (console_uart.reg == -1U)
		return get_console();

	/* otherwise just return the current information */
	return &console;
}
