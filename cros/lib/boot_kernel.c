/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <part.h>
#include <chromeos/boot_kernel.h>
#include <chromeos/common.h>
#include <chromeos/crossystem_data.h>

#include <vboot_api.h>

#define PREFIX "boot_kernel: "

/*
 * We uses a static variable to communicate with fit_update_fdt_before_boot().
 * For more information, please see commit log.
 */
static crossystem_data_t *g_crossystem_data = NULL;

/* defined in common/cmd_bootm.c */
int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* Maximum kernel command-line size */
#define CROS_CONFIG_SIZE	4096

/* Size of the x86 zeropage table */
#define CROS_PARAMS_SIZE	4096

/* Extra buffer to string replacement */
#define EXTRA_BUFFER		4096

/**
 * This loads kernel command line from the buffer that holds the loaded kernel
 * image. This function calculates the address of the command line from the
 * bootloader address.
 *
 * @param bootloader_address is the address of the bootloader in the buffer
 * @return kernel config address
 */
static char *get_kernel_config(char *bootloader_address)
{
	/* Use the bootloader address to find the kernel config location. */
	return bootloader_address - CROS_PARAMS_SIZE - CROS_CONFIG_SIZE;
}

static uint32_t get_dev_num(const block_dev_desc_t *dev)
{
	return dev->dev;
}

/* assert(0 <= val && val < 99); sprintf(dst, "%u", val); */
static char *itoa(char *dst, int val)
{
	if (val > 9)
		*dst++ = '0' + val / 10;
	*dst++ = '0' + val % 10;
	return dst;
}

/* copied from x86 bootstub code; sprintf(dst, "%02x", val) */
static void one_byte(char *dst, uint8_t val)
{
	dst[0] = "0123456789abcdef"[(val >> 4) & 0x0F];
	dst[1] = "0123456789abcdef"[val & 0x0F];
}

/* copied from x86 bootstub code; display a GUID in canonical form */
static char *emit_guid(char *dst, uint8_t *guid)
{
	one_byte(dst, guid[3]); dst += 2;
	one_byte(dst, guid[2]); dst += 2;
	one_byte(dst, guid[1]); dst += 2;
	one_byte(dst, guid[0]); dst += 2;
	*dst++ = '-';
	one_byte(dst, guid[5]); dst += 2;
	one_byte(dst, guid[4]); dst += 2;
	*dst++ = '-';
	one_byte(dst, guid[7]); dst += 2;
	one_byte(dst, guid[6]); dst += 2;
	*dst++ = '-';
	one_byte(dst, guid[8]); dst += 2;
	one_byte(dst, guid[9]); dst += 2;
	*dst++ = '-';
	one_byte(dst, guid[10]); dst += 2;
	one_byte(dst, guid[11]); dst += 2;
	one_byte(dst, guid[12]); dst += 2;
	one_byte(dst, guid[13]); dst += 2;
	one_byte(dst, guid[14]); dst += 2;
	one_byte(dst, guid[15]); dst += 2;
	return dst;
}

/**
 * This replaces:
 *   %D -> device number
 *   %P -> partition number
 *   %U -> GUID
 * in kernel command line.
 *
 * For example:
 *   ("root=/dev/sd%D%P", 2, 3)      -> "root=/dev/sdc3"
 *   ("root=/dev/mmcblk%Dp%P", 0, 5) -> "root=/dev/mmcblk0p5".
 *
 * @param src		Input string
 * @param devnum	Device number of the storage device we will mount
 * @param partnum	Partition number of the root file system we will mount
 * @param guid		GUID of the kernel partition
 * @param dst		Output string
 * @param dst_size	Size of output string
 * @return zero if it succeeds, non-zero if it fails
 */
static int update_cmdline(char *src, int devnum, int partnum, uint8_t *guid,
		char *dst, int dst_size)
{
	char *dst_end = dst + dst_size;
	int c;

	/* sanity check on inputs */
	if (devnum < 0 || devnum > 25 || partnum < 1 || partnum > 99 ||
			dst_size < 0 || dst_size > 10000) {
		VBDEBUG(PREFIX "insane input: %d, %d, %d\n", devnum, partnum,
				dst_size);
		return 1;
	}

	/*
	 * Condition "dst + X <= dst_end" checks if there is at least X bytes
	 * left in dst. We use X > 1 so that there is always 1 byte for '\0'
	 * after the loop.
	 *
	 * We constantly estimate how many bytes we are going to write to dst
	 * for copying characters from src or for the string replacements, and
	 * check if there is sufficient space.
	 */

#define CHECK_SPACE(bytes) \
	if (!(dst + (bytes) <= dst_end)) { \
		VBDEBUG(PREFIX "fail: need at least %d bytes\n", (bytes)); \
		return 1; \
	}

	while ((c = *src++)) {
		if (c != '%') {
			CHECK_SPACE(2);
			*dst++ = c;
			continue;
		}

		switch ((c = *src++)) {
		case '\0':
			VBDEBUG(PREFIX "mal-formed input: end in '%%'\n");
			return 1;
		case 'D':
			/*
			 * TODO: Do we have any better way to know whether %D
			 * is replaced by a letter or digits? So far, this is
			 * done by a rule of thumb that if %D is followed by a
			 * 'p' character, then it is replaced by digits.
			 */
			if (*src == 'p') {
				CHECK_SPACE(3);
				dst = itoa(dst, devnum);
			} else {
				CHECK_SPACE(2);
				*dst++ = 'a' + devnum;
			}
			break;
		case 'P':
			CHECK_SPACE(3);
			dst = itoa(dst, partnum);
			break;
		case 'U':
			/* GUID replacement needs 36 bytes */
			CHECK_SPACE(36 + 1);
			dst = emit_guid(dst, guid);
			break;
		default:
			CHECK_SPACE(3);
			*dst++ = '%';
			*dst++ = c;
			break;
		}
	}

#undef CHECK_SPACE

	*dst = '\0';
	return 0;
}

/* TODO Copy from tegra2-common.h so that coreboot can be built */
#ifndef QUOTE
/*
 * QUOTE(m) will evaluate to a string version of the value of the macro m
 * passed in.  The extra level of indirection here is to first evaluate the
 * macro m before applying the quoting operator.
 */
#define QUOTE_(m)	#m
#define QUOTE(m)	QUOTE_(m)
#endif

int boot_kernel(VbSelectAndLoadKernelParams *kparams, crossystem_data_t *cdata)
{
	/* sizeof(CHROMEOS_BOOTARGS) reserves extra 1 byte */
	char cmdline_buf[sizeof(CHROMEOS_BOOTARGS) + CROS_CONFIG_SIZE];
	/* Reserve EXTRA_BUFFER bytes for update_cmdline's string replacement */
	char cmdline_out[sizeof(CHROMEOS_BOOTARGS) + CROS_CONFIG_SIZE +
		EXTRA_BUFFER];
	char *cmdline;

	/* Chrome OS kernel has to be loaded at fixed location */
	char *argv[] = { "bootm", QUOTE(CHROMEOS_KERNEL_LOADADDR) };
	assert(kparams->kernel_buffer == (void *)CHROMEOS_KERNEL_LOADADDR);

	strcpy(cmdline_buf, CHROMEOS_BOOTARGS);

	/*
	 * casting bootloader_address of uint64_t type to uintptr_t before
	 * further casting it to char * to avoid compiler warning "cast to
	 * pointer from integer of different size" on 32-bit address machine.
	 */
	cmdline = get_kernel_config((char *)
			(uintptr_t)kparams->bootloader_address);
	/*
	 * strncat could write CROS_CONFIG_SIZE + 1 bytes to cmdline_buf. This
	 * is okay because the extra 1 byte has been reserved in sizeof().
	 */
	strncat(cmdline_buf, cmdline, CROS_CONFIG_SIZE);

	VBDEBUG(PREFIX "cmdline before update: ");
	VBDEBUG_PUTS(cmdline_buf);
	VBDEBUG_PUTS("\n");

	if (update_cmdline(cmdline_buf,
			get_dev_num(kparams->disk_handle),
			kparams->partition_number + 1,
			kparams->partition_guid,
			cmdline_out, sizeof(cmdline_out))) {
		VBDEBUG(PREFIX "failed replace %%[DUP] in command line\n");
		return 1;
	}

	setenv("bootargs", cmdline_out);
	VBDEBUG(PREFIX "cmdline after update:  ");
	VBDEBUG_PUTS(getenv("bootargs"));
	VBDEBUG_PUTS("\n");

	g_crossystem_data = cdata;

	do_bootm(NULL, 0, sizeof(argv)/sizeof(*argv), argv);

	VBDEBUG(PREFIX "failed to boot; is kernel broken?\n");
	return 1;
}

/*
 * This function does the last chance FDT update before booting to kernel.
 * Currently we modify the FDT by embedding crossystem data. So before
 * calling bootm(), g_crossystem_data should be set.
 */
int fit_update_fdt_before_boot(char *fdt, ulong *new_size)
{
	uint32_t ns;

	if (!g_crossystem_data) {
		VBDEBUG(PREFIX "warning: g_crossystem_data is NULL\n");
		return 0;
	}

	if (crossystem_data_embed_into_fdt(g_crossystem_data, fdt, &ns)) {
		VBDEBUG(PREFIX "crossystem_data_embed_into_fdt() failed\n");
		return 0;
	}

	*new_size = ns;
	return 0;
}
