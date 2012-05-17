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
#include <asm/arch/pinmux.h>

DECLARE_GLOBAL_DATA_PTR;

#define OM_STAT		(0x1f << 1)

/**
 * Copy data from SD or MMC device to RAM.
 *
 * @param offset	Block offset of the data
 * @param nblock	Number of blocks
 * @param dst		Destination address
 * @return 1 = True or 0 = False
 */
typedef u32 (*mmc_copy_func_t)(u32 offset, u32 nblock, u32 dst);

/**
 * Copy data from SPI flash to RAM.
 *
 * @param offset	Block offset of the data
 * @param nblock	Number of blocks
 * @param dst		Destination address
 * @return 1 = True or 0 = False
 */
typedef u32 (*spi_copy_func_t)(u32 offset, u32 nblock, u32 dst);


/**
 * Copy data through USB.
 *
 * @return 1 = True or 0 = False
 */
typedef u32 (*usb_copy_func_t)(void);

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
	unsigned int uboot_size;
	int is_cr_z_set;
	enum boot_mode boot_source;
	mmc_copy_func_t mmc_copy;

#if defined(CONFIG_EXYNOS_SPI_BOOT)
	spi_copy_func_t spi_copy;
#endif
	usb_copy_func_t usb_copy;

	uboot_size = exynos_get_uboot_size();
	boot_source = exynos_get_boot_device();

	/* Read iRAM location to check for secondary USB boot mode */
	sec_boot_check = readl(EXYNOS_IRAM_SECONDARY_BASE);
	if (sec_boot_check == EXYNOS_USB_SECONDARY_BOOT) {
		/*
		 * iROM needs program flow prediction to be disabled
		 * before copy from USB device to RAM
		 */
		is_cr_z_set = config_branch_prediction(0);
		usb_copy = *(usb_copy_func_t *)EXYNOS_COPY_USB_FNPTR_ADDR;
		usb_copy();
		config_branch_prediction(is_cr_z_set);
		return;
	}

	if (boot_source == BOOT_MODE_OM)
		boot_source = readl(EXYNOS_POWER_BASE) & OM_STAT;
	switch (boot_source) {
#if defined(CONFIG_EXYNOS_SPI_BOOT)
	case BOOT_MODE_SERIAL:
		spi_copy = *(spi_copy_func_t *)EXYNOS_COPY_SPI_FNPTR_ADDR;
		spi_copy(SPI_FLASH_UBOOT_POS, uboot_size,
				CONFIG_SYS_TEXT_BASE);
		break;
#endif
	case BOOT_MODE_MMC:
		mmc_copy = *(mmc_copy_func_t *)EXYNOS_COPY_MMC_FNPTR_ADDR;
		assert(!(uboot_size & 511));
		mmc_copy(BL2_START_OFFSET, uboot_size / 512,
				CONFIG_SYS_TEXT_BASE);
		break;
	default:
		panic("Invalid boot mode selection\n");
		break;
	}
}

/* The memzero function is not in SPL u-boot, so create one. */
void memzero(void *s, size_t n)
{
	char *ptr = s;
	size_t i;

	for (i = 0; i < n; i++)
		*ptr++ = '\0';
}

/*
 * Initialize the serial driver in SPL u-boot.
 * Besides the serial driver, it also setup the minimal set of its dependency,
 * like gd struct, pinmux, and serial.
 */
static void early_serial_init(void)
{
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;

	exynos_pinmux_config(EXYNOS_UART, PINMUX_FLAG_NONE);
	serial_init();
}

void board_init_f(unsigned long bootflag)
{
	__attribute__((noreturn)) void (*uboot)(void);

	gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);
	memzero((void *)gd, sizeof(gd_t));
	early_serial_init();

	printf("\n\nU-Boot SPL, board rev %u\n", board_get_revision());

	copy_uboot_to_ram();
	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	uboot();
	/* Never returns Here */
	panic("%s: u-boot jump failed", __func__);
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

/*
 * The following functions are required when linking console library to SPL.
 *
 * Enabling UART in SPL u-boot requires console library. But some
 * functions we needed in the console library depends on a bunch
 * of library in libgeneric, like lib/ctype.o, lib/div64.o, lib/string.o,
 * and lib/vsprintf.o. Adding them makes the SPL u-boot too large and not
 * fit into the expected size.
 *
 * So we mock these functions in SPL, i.e. vsprintf(), panic(), etc.,
 * in order to cut its dependency.
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
	char *str = buf, *s;
	ulong u;

	/*
	 * We won't implement all full functions of vsprintf().
	 * We only implement %s and %u, and ignore others and directly use
	 * the original format string as its result.
	 */

	while (*fmt) {
		if (*fmt != '%') {
			*str++ = *fmt++;
			continue;
		}
		fmt++;
		switch (*fmt) {
		case '%':
			*str++ = *fmt++;
			break;
		case 's':
			fmt++;
			s = va_arg(args, char *);
			while (*s)
				*str++ = *s++;
			break;
		case 'u':
			fmt++;
			u = va_arg(args, ulong);
			s = simple_itoa(u);
			while (*s)
				*str++ = *s++;
			break;
		default:
			/* Print the original string for unsupported formats */
			*str++ = '%';
			*str++ = *fmt++;
		}
	}
	*str = '\0';
	return str - buf;
}

void panic(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	putc('\n');
	va_end(args);
#if defined(CONFIG_PANIC_HANG)
	hang();
#else
	udelay(100000);		/* allow messages to go out */
	do_reset(NULL, 0, 0, NULL);
#endif
	while (1)
		;
}

void __assert_fail(const char *assertion, const char *file, unsigned line,
		const char *function)
{
	/* This will not return */
	panic("%s:%u: %s: Assertion `%s' failed.", file, line, function,
			assertion);
}

char *simple_itoa(ulong i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	static char local[22];
	char *p = &local[21];

	*p-- = '\0';
	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);
	return p + 1;
}
