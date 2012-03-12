/*
 * Copyright (C) 2012 Samsung Electronics
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
#include <fdtdec.h>
#include <asm/io.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_OF_CONTROL
static int fdtdec_decode_smc911x(const void *blob, struct fdt_smc911x *config)
{
	int err;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS5_SROM);

	config->bank = fdtdec_get_int(blob, node, "bank", 0);

	err = fdtdec_get_int_array(blob, node, "srom-timing", config->timing,
			FDT_SROM_TIMING_COUNT);
	if (err < 0) {
		debug("missing srom timing info\n");
		return -FDT_ERR_NOTFOUND;
	}

	return 0;
}
#endif

#ifdef CONFIG_SMC911X
static void smc9115_pre_init(void)
{
	u32 smc_bw_conf, smc_bc_conf;
	struct fdt_smc911x smc911x_config;

	exynos_pinmux_config(EXYNOS_SMC911X, PINMUX_FLAG_NONE);

#ifdef CONFIG_OF_CONTROL
	if (fdtdec_decode_smc911x(gd->fdt_blob, &smc911x_config) < 0) {
		debug("ethernet configuration failed\n");
		return;
	}
#else
	/* Non-FDT configuration - bank number and timing parameters*/
	smc911x_config.bank = CONFIG_ENV_SROM_BANK;

	smc911x_config.timing[FDT_SROM_TACS] = 0x01;
	smc911x_config.timing[FDT_SROM_TCOS] = 0x01;
	smc911x_config.timing[FDT_SROM_TACC] = 0x06;
	smc911x_config.timing[FDT_SROM_TCOH] = 0x01;
	smc911x_config.timing[FDT_SROM_TAH] = 0x0C;
	smc911x_config.timing[FDT_SROM_TACP] = 0x09;
	smc911x_config.timing[FDT_SROM_PMC] = 0x01;
#endif

	/* Ethernet needs data bus width of 16 bits */
	smc_bw_conf = SROMC_DATA16_WIDTH(smc911x_config.bank)
			| SROMC_BYTE_ENABLE(smc911x_config.bank);

	smc_bc_conf = SROMC_BC_TACS(smc911x_config.timing[FDT_SROM_TACS])   |\
			SROMC_BC_TCOS(smc911x_config.timing[FDT_SROM_TCOS]) |\
			SROMC_BC_TACC(smc911x_config.timing[FDT_SROM_TACC]) |\
			SROMC_BC_TCOH(smc911x_config.timing[FDT_SROM_TCOH]) |\
			SROMC_BC_TAH(smc911x_config.timing[FDT_SROM_TAH])   |\
			SROMC_BC_TACP(smc911x_config.timing[FDT_SROM_TACP]) |\
			SROMC_BC_PMC(smc911x_config.timing[FDT_SROM_PMC]);

	/* Select and configure the SROMC bank */
	s5p_config_sromc(smc911x_config.bank, smc_bw_conf, smc_bc_conf);
}
#endif

int fdtdec_decode_memory(const void *blob, struct fdt_memory *config)
{
	int node, len;
	const fdt_addr_t *cell;

	node = fdt_path_offset(blob, "/memory");
	if (node < 0) {
		debug("Could not find the path /memory\n");
		return node;
	}
	cell = fdt_getprop(blob, node, "reg", &len);
	if (cell && len == sizeof(fdt_addr_t) * 2) {
		config->start = fdt_addr_to_cpu(cell[0]);
		config->end = fdt_addr_to_cpu(cell[1]);
	} else {
		return -FDT_ERR_BADLAYOUT;
	}

	return 0;
}

int board_init(void)
{
	struct fdt_memory mem_config;

	if (fdtdec_decode_memory(gd->fdt_blob, &mem_config)) {
		debug("%s: Failed to decode memory\n", __func__);
		return -1;
	}

	gd->bd->bi_boot_params = mem_config.start + 0x100UL;

#ifdef CONFIG_OF_CONTROL
	gd->bd->bi_arch_number = fdtdec_get_config_int(gd->fdt_blob,
				"machine-arch-id", -1);
	if (gd->bd->bi_arch_number == -1U)
		debug("Warning: No /config/machine-arch-id defined in fdt\n");
#endif
	return 0;
}

int dram_init(void)
{
	struct fdt_memory mem_config;

	if (fdtdec_decode_memory(gd->fdt_blob, &mem_config)) {
		debug("%s: Failed to decode memory\n", __func__);
		return -1;
	}

	gd->ram_size = get_ram_size((long *)mem_config.start,
				mem_config.end);
	return 0;
}

void dram_init_banksize(void)
{
	struct fdt_memory mem_config;

	if (fdtdec_decode_memory(gd->fdt_blob, &mem_config)) {
		debug("%s: Failed to decode memory\n", __func__);
		return;
	}

	gd->bd->bi_dram[0].start = mem_config.start;
	gd->bd->bi_dram[0].size = get_ram_size((long *)mem_config.start,
				mem_config.end);
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	smc9115_pre_init();
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
#ifdef CONFIG_OF_CONTROL
	const char *board_name;

	board_name = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	if (board_name == NULL)
		printf("\nUnknown Board\n");
	else
		printf("\nBoard: %s\n", board_name);
#else
	printf("\nBoard: SMDK5250\n");
#endif

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	exynos_pinmux_config(EXYNOS_SDMMC2, PINMUX_FLAG_NONE);
#ifdef CONFIG_OF_CONTROL
	return s5p_mmc_init(gd->fdt_blob);
#else
	return s5p_mmc_init(2, 4);
#endif
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	exynos_pinmux_config(EXYNOS_UART, PINMUX_FLAG_NONE);
	return 0;
}
#endif
