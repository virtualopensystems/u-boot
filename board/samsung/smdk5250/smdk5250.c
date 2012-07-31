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
#include <asm/arch/sata.h>
#include <asm/arch/exynos-tmu.h>
#include <asm/arch/exynos-cpufreq.h>

#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

struct local_info {
	struct mkbp_dev *mkbp_dev;	/* Pointer to mkbp device */
	int arbitrate_node;
	struct fdt_gpio_state ap_claim;
	struct fdt_gpio_state ec_claim;

	/* Time between requesting bus and deciding that we have it */
	unsigned slew_delay_us;

	/* Time between retrying to see if the AP has released the bus */
	unsigned wait_retry_us;

	/* Time to wait until the bus becomes free */
	unsigned wait_free_ms;
};

static struct local_info local;
static uint32_t cpufreq_loop_count;

/*
 * Called to do the needful when tstc has a character ready
 * Meant to work in contrast to board_poll_devices
 */
void board_tstc_ready(void)
{
#ifdef CONFIG_EXYNOS_CPUFREQ
	if (cpufreq_loop_count >= 10000000) {
		/* Character received, increase ARM frequency */
		exynos5250_set_frequency(CPU_FREQ_L1700);
	}
	cpufreq_loop_count = 0;
#endif /* CONFIG_EXYNOS_CPUFREQ */
}

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
		break;
	default:
		debug("Unknown TMU state\n");
	}
#endif /* CONFIG_EXYNOS_TMU */
#ifdef CONFIG_EXYNOS_CPUFREQ
	cpufreq_loop_count++;
	if (cpufreq_loop_count == 10000000) {
		/* User is idle, decrease ARM frequency*/
		exynos5250_set_frequency(CPU_FREQ_L200);
	}
#endif /* CONFIG_EXYNOS_CPUFREQ */
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
	return local.mkbp_dev;
}

static int board_init_mkbp_devices(const void *blob)
{
	local.mkbp_dev = mkbp_init(blob);
	if (!local.mkbp_dev) {
		debug("%s: cannot init mkbp device\n", __func__);
		return -1;
	}

	return 0;
}

static int board_i2c_arb_init(const void *blob)
{
	int node;

	local.arbitrate_node = -1;
	node = fdtdec_next_compatible(blob, 0, COMPAT_GOOGLE_ARBITRATOR);
	if (node < 0) {
		debug("Cannot find bus arbitrator node\n");
		return 0;
	}

	if (fdtdec_decode_gpio(blob, node, "google,ap-claim-gpios",
				&local.ap_claim) ||
			fdtdec_decode_gpio(blob, node, "google,ec-claim-gpios",
				&local.ec_claim)) {
		debug("Cannot find bus arbitrator GPIOs\n");
		return 0;
	}

	if (fdtdec_setup_gpio(&local.ap_claim) ||
			fdtdec_setup_gpio(&local.ec_claim)) {
		debug("Cannot claim arbitration GPIOs\n");
		return -1;
	}

	/* We are currently not claiming the bus */
	gpio_direction_output(local.ap_claim.gpio, 1);
	gpio_direction_input(local.ec_claim.gpio);
	gpio_set_pull(local.ec_claim.gpio, EXYNOS_GPIO_PULL_UP);

	local.arbitrate_node = fdtdec_lookup_phandle(blob, node,
						     "google,arbitrate-bus");
	if (local.arbitrate_node < 0) {
		debug("Cannot find bus to arbitrate\n");
		return -1;
	}

	local.slew_delay_us = fdtdec_get_int(blob, node,
					     "google,slew-delay-us", 10);
	local.wait_retry_us = fdtdec_get_int(blob, node,
					     "google,wait-retry-us", 2000);
	local.wait_free_ms = fdtdec_get_int(blob, node,
					    "google,wait-free-us", 50000);
	local.wait_free_ms = DIV_ROUND_UP(local.wait_free_ms, 1000);
	debug("Bus arbitration ready on fdt node %d\n", local.arbitrate_node);

	return 0;
}

/**
 * Fix-up the kernel device tree so the bridge pd_n and rst_n gpios accurately
 * reflect the current board rev.
 */
void ft_board_setup(void *blob, bd_t *bd)
{
	int ret, rev, np, len;
	const struct fdt_property *prop;

	/* Do nothing for newer boards */
	rev = board_get_revision();
	if (rev < 4 || rev == 6)
		return;

	/*
	 * If this is an older board, replace pd_n_gpio contents with that of
	 * rst_n_gpio and delete rst_n_gpio from the dt.
	 */
	np = fdtdec_next_compatible(blob, 0, COMPAT_NXP_PTN3460);
	if (np < 0) {
		debug("%s: Could not find COMPAT_NXP_PTN3460\n", __func__);
		return;
	}

	prop = fdt_get_property(blob, np, "rst_n_gpio", &len);
	if (!prop) {
		debug("%s: Could not get property err=%d\n", __func__, len);
		return;
	}

	ret = fdt_setprop_inplace(blob, np, "pd_n_gpio", prop->data,
			len);
	if (ret) {
		debug("%s: Could not setprop inplace err=%d\n", __func__, ret);
		return;
	}

	ret = fdt_delprop(blob, np, "rst_n_gpio");
	if (ret) {
		debug("%s: Could not delprop err=%d\n", __func__, ret);
		return;
	}
}

/**
 * Enable the eDP to LVDS bridge.
 *
 * @return 0 if ok, non-zero if error
 */
static int board_dp_enable_bridge(const void *blob)
{
	const int NUM_TRIES = 10;
	int ret, np, rev, i;
	struct fdt_gpio_state pd, rst, hpd;

	np = fdtdec_next_compatible(blob, 0, COMPAT_NXP_PTN3460);
	if (np < 0) {
		debug("%s: Could not find COMPAT_NXP_PTN3460 (%d)\n", __func__,
			ret);
		return np;
	}
	ret = fdtdec_decode_gpio(blob, np, "pd_n_gpio", &pd);
	if (ret) {
		debug("%s: Could not decode pd_n_gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_decode_gpio(blob, np, "rst_n_gpio", &rst);
	if (ret) {
		debug("%s: Could not decode rst_n_gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_decode_gpio(blob, np, "hotplug-gpio", &hpd);
	if (ret) {
		debug("%s: Could not decode hotplug (%d)\n", __func__, ret);
		return ret;
	}

	/* If board is older, replace pd gpio with rst gpio */
	rev = board_get_revision();
	if (rev >= 4 && rev != 6) {
		pd = rst;
		rst.gpio = FDT_GPIO_NONE;
	}

	/* Setup the GPIOs */
	ret = fdtdec_setup_gpio(&pd);
	if (ret) {
		debug("%s: Could not setup pd gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_setup_gpio(&rst);
	if (ret) {
		debug("%s: Could not setup rst gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_setup_gpio(&hpd);
	if (ret) {
		debug("%s: Could not setup hpd gpio (%d)\n", __func__, ret);
		return ret;
	}

	/* Mux HPHPD to the special hotplug detect mode */
	exynos_pinmux_config(PERIPH_ID_DPHPD, 0);

	/* De-assert PD (and possibly RST) to power up the bridge */
	fdtdec_set_gpio(&pd, 0);
	gpio_cfg_pin(pd.gpio, EXYNOS_GPIO_OUTPUT);
	gpio_set_pull(pd.gpio, EXYNOS_GPIO_PULL_NONE);
	if (fdt_gpio_isvalid(&rst)) {
		fdtdec_set_gpio(&rst, 0);
		gpio_cfg_pin(rst.gpio, EXYNOS_GPIO_OUTPUT);
		gpio_set_pull(rst.gpio, EXYNOS_GPIO_PULL_NONE);
	}

	/*
	 * We'll wait for the bridge to come up here and retry if it didn't.
	 *
	 * Sometimes the retry works and sometimes things are really wedged
	 * and retry doesn't work.
	 *
	 * We really want to do better in the long term.  Ideally we should
	 * set the PD pin high very early at boot and then probe the HPD pin
	 * to see when the chip has powered up.
	 *
	 * Right now we don't do the "ideal" solution for a few reasons:
	 * - There is a phantom "high" on the HPD chip during its bootup.  The
	 *   phantom high comes within 7ms of de-asserting PD and persists
	 *   for at least 15ms.  The real high comes roughly 50ms after
	 *   PD is de-asserted.  The phantom high makes it hard for us to
	 *   know when the NXP chip is up.
	 * - We're trying to figure out how to make it so that the retry isn't
	 *   needed, so we don't want to architect things too much until we
	 *   get a more final solution.
	 */
	for (i = 0; i < NUM_TRIES; i++) {
		/* Hardcode 90ms (max powerup) so we know HPD is valid. */
		mdelay(90);

		/* Check HPD.  If it's high, we're all good. */
		if (fdtdec_get_gpio(&hpd))
			return 0;

		/*
		 * If we're here, the bridge chip failed to initialize.
		 * Drive DP_N low in an attempt to reset.
		 *
		 * Arbitrarily wait 300ms here with DP_N low.  Don't know for
		 * sure how long we should wait, but we're being paranoid.
		 */
		debug("%s: eDP bridge failed to come up; try %d of %d\n",
		       __func__, i+1, NUM_TRIES);
		fdtdec_set_gpio(&pd, 1);
		fdtdec_set_gpio(&rst, 1);
		mdelay(300);
		fdtdec_set_gpio(&pd, 0);
		fdtdec_set_gpio(&rst, 0);
	}
	debug("%s: eDP bridge failed to come up; giving up\n", __func__);
	return -1;
}

void board_lcd_enable(void)
{
	/*
	 * This delay is T3 in the LCD timing spec (defined as > 200ms). We need
	 * to  set this above 260ms since the bridge takes 50-60ms from when it
	 * asserts HP to when it starts driving LVDS clock/data
	 */
	mdelay(260);

	tps65090_fet_enable(1); /* Enable FET1, backlight */

	/* This delay is T5 in the LCD timing spec (defined as > 10ms) */
	mdelay(10);

	/*
	 * Configure backlight PWM as a simple output high (100% brightness)
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_direction_output(GPIO_B20, 1);

	/* This delay is T6 in the LCD timing spec (defined as > 10ms) */
	mdelay(10);

	/*
	 * Configure GPIO for LCD_BL_EN
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_direction_output(GPIO_X30, 1);
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

	if (board_i2c_arb_init(gd->fdt_blob))
		return -1;

	board_i2c_init(gd->fdt_blob);

/* Enable power for LCD */
#ifdef CONFIG_TPS65090_POWER
	tps65090_init();
	tps65090_fet_enable(6); /* Enable FET6, lcd panel */
#endif

	if (max77686_enable_32khz_cp()) {
		debug("%s: Failed to enable max77686 32khz coprocessor clock\n",
				 __func__);
		return -1;
	}

#if defined CONFIG_EXYNOS_CPUFREQ
	if (exynos5250_cpufreq_init(gd->fdt_blob)) {
		debug("%s: Failed to init CPU frequency scaling\n", __func__);
		return -1;
	}
#endif

#if defined CONFIG_EXYNOS_TMU
	if (tmu_init(gd->fdt_blob)) {
		debug("%s: Failed to init TMU\n", __func__);
		return -1;
	}
#endif

	if (board_dp_enable_bridge(gd->fdt_blob))
		debug("%s: Could not enable dp bridge\n", __func__);

	if (board_init_mkbp_devices(gd->fdt_blob))
		return -1;

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

void board_i2c_release_bus(int node)
{
	/* If this is us, release the bus */
	if (node == local.arbitrate_node) {
		gpio_set_value(local.ap_claim.gpio, 1);
		udelay(local.slew_delay_us);
	}
}

int board_i2c_claim_bus(int node)
{
	unsigned start;

	if (node != local.arbitrate_node)
		return 0;

// 	putc('c');

	/* Start a round of trying to claim the bus */
	start = get_timer(0);
	do {
		unsigned start_retry;
		int waiting = 0;

		/* Indicate that we want to claim the bus */
		gpio_set_value(local.ap_claim.gpio, 0);
		udelay(local.slew_delay_us);

		/* Wait for the EC to release it */
		start_retry = get_timer(0);
		while (get_timer(start_retry) < local.wait_retry_us) {
			if (gpio_get_value(local.ec_claim.gpio)) {
				/* We got it, so return */
				return 0;
			}

			if (!waiting) {
				waiting = 1;
			}
		}

		/* It didn't release, so give up, wait, and try again */
		gpio_set_value(local.ap_claim.gpio, 1);

		udelay(local.wait_retry_us);
	} while (get_timer(start) < local.wait_free_ms);

	/* Give up, release our claim */
	printf("I2C: Could not claim bus, timeout %lu\n", get_timer(start));

	return -1;
}

#ifdef CONFIG_SATA_AHCI
int sata_initialize(void)
{
	return exynos5_sata_init(gd->fdt_blob);
}
#endif
