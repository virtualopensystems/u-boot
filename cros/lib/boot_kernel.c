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
 * @param src - input string
 * @param devnum - device number of the storage device we will mount
 * @param partnum - partition number of the root file system we will mount
 * @param guid - guid of the kernel partition
 * @param dst - output string; a copy of [src] with special characters replaced
 */
static void update_cmdline(char *src, int devnum, int partnum, uint8_t *guid,
		char *dst)
{
	int c;

	// sanity check on inputs
	if (devnum < 0 || devnum > 25 || partnum < 1 || partnum > 99) {
		VBDEBUG(PREFIX "insane input: %d, %d\n", devnum, partnum);
		devnum = 0;
		partnum = 3;
	}

	while ((c = *src++)) {
		if (c != '%') {
			*dst++ = c;
			continue;
		}

		switch ((c = *src++)) {
		case '\0':
			/* input ends in '%'; is it not well-formed? */
			src--;
			break;
		case 'D':
			/*
			 * TODO: Do we have any better way to know whether %D
			 * is replaced by a letter or digits? So far, this is
			 * done by a rule of thumb that if %D is followed by a
			 * 'p' character, then it is replaced by digits.
			 */
			if (*src == 'p')
				dst = itoa(dst, devnum);
			else
				*dst++ = 'a' + devnum;
			break;
		case 'P':
			dst = itoa(dst, devnum);
			break;
		case 'U':
			dst = emit_guid(dst, guid);
			break;
		default:
			*dst++ = '%';
			*dst++ = c;
			break;
		}
	}

	*dst = '\0';
}

int boot_kernel(VbSelectAndLoadKernelParams *kparams, crossystem_data_t *cdata)
{
	/* sizeof(CHROMEOS_BOOTARGS) reserves extra 1 byte */
	char cmdline_buf[sizeof(CHROMEOS_BOOTARGS) + CROS_CONFIG_SIZE];
	/* Reserve EXTRA_BUFFER bytes for update_cmdline's string replacement */
	char cmdline_out[sizeof(CHROMEOS_BOOTARGS) + CROS_CONFIG_SIZE +
		EXTRA_BUFFER];
	char load_address[32];
	char *argv[2] = {"bootm", load_address};
	char *cmdline;

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

	VBDEBUG(PREFIX "cmdline before update: %s\n", cmdline_buf);

	/* TODO fix potential buffer overflow */
	update_cmdline(cmdline_buf,
			get_dev_num(kparams->disk_handle),
			kparams->partition_number + 1,
			kparams->partition_guid,
			cmdline_out);

	setenv("bootargs", cmdline_out);
	VBDEBUG(PREFIX "cmdline after update:  %s\n", getenv("bootargs"));

	g_crossystem_data = cdata;

	sprintf(load_address, "0x%p", kparams->kernel_buffer);
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
