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
#include <edid.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <i2c.h>
#include <max77686.h>
#include <mkbp.h>
#include <mmc.h>
#include <netdev.h>
#include <tps65090.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ehci-s5p.h>
#include <asm/arch/board.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/mshc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>
#include <asm/arch-exynos5/power.h>
#include <asm/arch/exynos-tmu.h>

#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

static struct mkbp_dev *mkbp_dev;	/* Pointer to mkbp device */

/*
 * Polling various devices on board for details and status monitoring purposes
 */
void board_poll_devices(void)
{
#if defined CONFIG_EXYNOS_TMU
	int temp;

	switch (tmu_monitor(&temp)) {
	case TMU_STATUS_TRIPPED:
		puts("EXYNOS_TMU: TRIPPING! Device power going down ...\n");
		power_shutdown();
		break;
	case TMU_STATUS_WARNING:
		puts("EXYNOS_TMU: WARNING! Temperature very high\n");
		break;
	case TMU_STATUS_INIT:
	case TMU_STATUS_NORMAL:
		debug("syetm is in normal temperature state\n");
		break;
	default:
		debug("Unknown TMU state\n");
	}
#endif
}

#ifdef CONFIG_OF_CONTROL
static int decode_sromc(const void *blob, struct fdt_sromc *config)
{
	int err;
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS5_SROMC);
	if (node < 0) {
		debug("Could not find SROMC node\n");
		return node;
	}

	config->bank = fdtdec_get_int(blob, node, "bank", 0);
	config->width = fdtdec_get_int(blob, node, "width", 2);

	err = fdtdec_get_int_array(blob, node, "srom-timing", config->timing,
			FDT_SROM_TIMING_COUNT);
	if (err < 0) {
		debug("Could not decode SROMC configuration\n");
		return -FDT_ERR_NOTFOUND;
	}

	return 0;
}

/**
 * Get the EDID data from the display device
 *
 * @param blob		Device tree blob
 * @param node		The display node
 * @param edid_buf	Returns the EDID data
 * @return 0 on success, or -1 on error
 */
static int get_edid_from_lcd(const void *blob, int node,
		struct edid1_info *edid_buf)
{
	int i2c_bus, slave_addr, parent;

	slave_addr = fdtdec_get_addr(blob, node, "reg");
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}
	i2c_bus = i2c_get_bus_num_fdt(blob, parent);
	if (i2c_bus < 0)
		return -1;

	if (i2c_set_bus_num(i2c_bus))
		return -1;
	if (i2c_read(slave_addr, 0, 1, (uchar *)edid_buf, sizeof(*edid_buf)))
		return -1;

	return 0;
}

/**
 * Check the EDID in the FDT valid or not. If not, get it from the real device.
 *
 * @param blob	Device tree blob
 * @return 0 on success, or -1 on error
 */
static int fdt_check_edid(const void *blob)
{
	int node, len;
	struct edid1_info *edid;
	struct edid1_info edid_buf;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS_LCD);
	if (node < 0) {
		debug("Could not find LCD node\n");
		return -1;
	}

	edid = (struct edid1_info *)fdt_getprop(blob, node, "edid", &len);
	if (edid_check_info(edid)) {
		debug("%s: Get the EDID from the real device\n", __func__);
		if (get_edid_from_lcd(blob, node, &edid_buf))
			return -1;
		edid = &edid_buf;
	}
#ifdef DEBUG
	edid_print_info(edid);
#endif
	return 0;
}
#endif

uint32_t exynos5_read_and_clear_spl_marker(void)
{
	uint32_t value, *marker = (uint32_t *)CONFIG_IRAM_STACK;

	/* Stack grows toward lower address of memory */
	value = marker[-1];
	marker[-1] = 0;

	return value;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	u32 smc_bw_conf, smc_bc_conf;
	struct fdt_sromc config;
	fdt_addr_t base_addr;
	int node;

#ifdef CONFIG_OF_CONTROL
	node = decode_sromc(gd->fdt_blob, &config);
	if (node < 0)
		return -1;
	node = fdtdec_next_compatible(gd->fdt_blob, node, COMPAT_SMSC_LAN9215);
	if (node < 0) {
		debug("%s: Could not find lan9215 configuration\n", __func__);
		return -1;
	}
	base_addr = fdtdec_get_addr(gd->fdt_blob, node, "reg");
	if (base_addr == FDT_ADDR_T_NONE) {
		debug("%s: Could not find lan9215 address\n", __func__);
		return -1;
	}
#else
	/* Non-FDT configuration - bank number and timing parameters*/
	config.bank = CONFIG_ENV_SROM_BANK;
	config.width = 2;

	config.timing[FDT_SROM_TACS] = 0x01;
	config.timing[FDT_SROM_TCOS] = 0x01;
	config.timing[FDT_SROM_TACC] = 0x06;
	config.timing[FDT_SROM_TCOH] = 0x01;
	config.timing[FDT_SROM_TAH] = 0x0C;
	config.timing[FDT_SROM_TACP] = 0x09;
	config.timing[FDT_SROM_PMC] = 0x01;
	base_addr = CONFIG_SMC911X_BASE;
#endif

	/* Ethernet needs data bus width of 16 bits */
	if (config.width != 2) {
		debug("%s: Unsupported bus width %d\n", __func__,
			config.width);
		return -1;
	}
	smc_bw_conf = SROMC_DATA16_WIDTH(config.bank)
			| SROMC_BYTE_ENABLE(config.bank);

	smc_bc_conf = SROMC_BC_TACS(config.timing[FDT_SROM_TACS])   |\
			SROMC_BC_TCOS(config.timing[FDT_SROM_TCOS]) |\
			SROMC_BC_TACC(config.timing[FDT_SROM_TACC]) |\
			SROMC_BC_TCOH(config.timing[FDT_SROM_TCOH]) |\
			SROMC_BC_TAH(config.timing[FDT_SROM_TAH])   |\
			SROMC_BC_TACP(config.timing[FDT_SROM_TACP]) |\
			SROMC_BC_PMC(config.timing[FDT_SROM_PMC]);

	/* Select and configure the SROMC bank */
	exynos_pinmux_config(PERIPH_ID_SROMC, config.bank | PINMUX_FLAG_16BIT);
	s5p_config_sromc(config.bank, smc_bw_conf, smc_bc_conf);
	return smc911x_initialize(0, base_addr);
#endif
	return 0;
}

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

int board_usb_vbus_init(void)
{
	/* Enable VBUS power switch */
	gpio_direction_output(GPIO_X11, 1);
	/* VBUS turn ON time */
	mdelay(3);

	return 0;
}

struct mkbp_dev *board_get_mkbp_dev(void)
{
	return mkbp_dev;
}

static int board_init_mkbp_devices(const void *blob)
{
	mkbp_dev = mkbp_init(blob);
	if (!mkbp_dev) {
		debug("%s: cannot init mkbp device\n", __func__);
		return -1;
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
#ifdef CONFIG_EXYNOS_SPI
	spi_init();
#endif

	board_i2c_init(gd->fdt_blob);

/* Enable power for LCD */
#ifdef CONFIG_TPS65090_POWER
	tps65090_init();
	tps65090_fet_enable(1); /* Enable FET1, backlight */
	tps65090_fet_enable(6); /* Enable FET6, lcd panel */
#endif

	if (max77686_enable_32khz_cp()) {
		debug("%s: Failed to enable max77686 32khz coprocessor clock\n",
				 __func__);
		return -1;
	}

#if defined CONFIG_EXYNOS_TMU
	if (tmu_init(gd->fdt_blob)) {
		debug("%s: Failed to init TMU\n", __func__);
		return -1;
	}
#endif

	/*
	 * Configure backlight PWM as a simple output high (100% brightness)
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_cfg_pin(GPIO_B20, GPIO_OUTPUT);
	gpio_set_value(GPIO_B20, 1);

	/*
	 * Configure GPIO for LCD_BL_EN
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_cfg_pin(GPIO_X30, GPIO_OUTPUT);
	gpio_set_value(GPIO_X30, 1);

	if (board_init_mkbp_devices(gd->fdt_blob))
		return -1;

	if (fdt_check_edid(gd->fdt_blob)) {
		debug("%s: Failed to get a correct EDID\n", __func__);
		return -1;
	}

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

int board_get_revision(void)
{
	struct fdt_gpio_state gpios[CONFIG_BOARD_REV_GPIO_COUNT];
	unsigned gpio_list[CONFIG_BOARD_REV_GPIO_COUNT];
	int board_rev = -1;
	int count = 0;
	int node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
				      COMPAT_GOOGLE_BOARD_REV);
	if (node >= 0) {
		count = fdtdec_decode_gpios(gd->fdt_blob, node,
				"google,board-rev-gpios", gpios,
				CONFIG_BOARD_REV_GPIO_COUNT);
	}
	if (count > 0) {
		int i;

		for (i = 0; i < count; i++)
			gpio_list[i] = gpios[i].gpio;
		board_rev = gpio_decode_number(gpio_list, count);
	} else {
		debug("%s: No board revision information in fdt\n", __func__);
	}

	return board_rev;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
#ifdef CONFIG_OF_CONTROL
	const char *board_name;

	board_name = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("\nBoard: %s, rev %d\n", board_name ? board_name : "<unknown>",
	       board_get_revision());
#else
	printf("\nBoard: SMDK5250\n");
#endif

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_getcd(struct mmc *mmc)
{
	struct mshci_host *host = mmc->priv;
	int present = 1; /* for ch0 (eMMC) card is always present */

	if (host->peripheral == PERIPH_ID_SDMMC2)
		present = !readl(&host->reg->cdetect);

	return present;
}

int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_S5P_MSHCI
	s5p_mshci_init(gd->fdt_blob);
#endif
	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	exynos_pinmux_config(EXYNOS_UART, PINMUX_FLAG_NONE);
	return 0;
}
#endif
