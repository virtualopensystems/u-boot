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
#include <mkbp_tps65090.h>
#include <mmc.h>
#include <netdev.h>
#include <s5m8767.h>
#include <tps65090.h>
#include <errno.h>
#include <lcd.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ehci-s5p.h>
#include <asm/arch/board.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/mshc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <asm/arch-exynos5/power.h>
#include <asm/arch/sata.h>
#include <asm/arch/exynos-tmu.h>
#include <asm/arch/exynos-cpufreq.h>
#include <asm/arch/s5p-dp.h>

#include "board.h"

#ifndef CONFIG_EXYNOS_DISPLAYPORT
#error "CONFIG_EXYNOS_DISPLAYPORT must be defined for smdk5250"
#endif

DECLARE_GLOBAL_DATA_PTR;

struct local_info {
	struct mkbp_dev *mkbp_dev;	/* Pointer to mkbp device */
	int mkbp_err;			/* Error for mkbp, 0 if ok */
	int arbitrate_node;
	struct fdt_gpio_state ap_claim;
	struct fdt_gpio_state ec_claim;

	/* DisplayPort gpios */
	struct fdt_gpio_state dp_pd;
	struct fdt_gpio_state dp_rst;
	struct fdt_gpio_state dp_hpd;

	/* Time between requesting bus and deciding that we have it */
	unsigned slew_delay_us;

	/* Time between retrying to see if the AP has released the bus */
	unsigned wait_retry_ms;

	/* Time to wait until the bus becomes free */
	unsigned wait_free_ms;

	/* TPschrome and battery are behind the EC */
	int ec_passthrough;
	/* PMIC chip : COMPAT_MAXIM_MAX77686 or COMPAT_SAMSUNG_S5M8767 */
	enum fdt_compat_id pmic;
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

/**
 * Read and clear the marker value; then return the read value.
 *
 * This marker is set to EXYNOS5_SPL_MARKER when SPL runs. Then in U-Boot
 * we can check (and clear) this marker to see if we were run from SPL.
 * If we were called from another U-Boot, the marker will be clear.
 *
 * @return marker value (EXYNOS5_SPL_MARKER if we were run from SPL, else 0)
 */
static uint32_t exynos5_read_and_clear_spl_marker(void)
{
	uint32_t value, *marker = (uint32_t *)CONFIG_SPL_MARKER;

	value = *marker;
	*marker = 0;

	return value;
}

int board_is_processor_reset(void)
{
	static uint8_t inited, is_reset;
	uint32_t marker_value;

	if (!inited) {
		marker_value = exynos5_read_and_clear_spl_marker();
		is_reset = marker_value == EXYNOS5_SPL_MARKER;
		inited = 1;
	}

	return is_reset;
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
	if (node < 0) {
		debug("%s: Could not find sromc configuration\n", __func__);
		return 0;
	}
	node = fdtdec_next_compatible(gd->fdt_blob, node, COMPAT_SMSC_LAN9215);
	if (node < 0) {
		debug("%s: Could not find lan9215 configuration\n", __func__);
		return 0;
	}

	/* We now have a node, so any problems from now on are errors */
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

/*
 * This functions disable the USB3.0 PLL to save power
 */
static void disable_usb30_pll(void)
{
	int node, ret;
	struct fdt_gpio_state en_gpio;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_SAMSUNG_EXYNOS_USB);
	if (node < 0)
		return;

	ret = fdtdec_decode_gpio(gd->fdt_blob, node, "usb3-pll-gpio", &en_gpio);
	if (ret)
		return;

	fdtdec_setup_gpio(&en_gpio);
	gpio_direction_output(en_gpio.gpio, en_gpio.flags);
}

static int board_init_mkbp_devices(const void *blob)
{
	local.mkbp_err = mkbp_init(blob, &local.mkbp_dev);
	if (local.mkbp_err)
		return -1;  /* Will report in board_late_init() */

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
	local.wait_retry_ms = fdtdec_get_int(blob, node,
					     "google,wait-retry-us", 2000);
	local.wait_retry_ms = DIV_ROUND_UP(local.wait_retry_ms, 1000);
	local.wait_free_ms = fdtdec_get_int(blob, node,
					    "google,wait-free-us", 50000);
	local.wait_free_ms = DIV_ROUND_UP(local.wait_free_ms, 1000);
	debug("Bus arbitration ready on fdt node %d\n", local.arbitrate_node);

	return 0;
}

int board_get_pmic(void)
{
	int ret;

	if (local.pmic == COMPAT_UNKNOWN) {
		i2c_set_bus_num(0);
		ret = i2c_probe(S5M8767_I2C_ADDR);
		if (!ret) {
			local.pmic = COMPAT_SAMSUNG_S5M8767;
		} else {
			ret = i2c_probe(MAX77686_I2C_ADDR);
			if (!ret)
				local.pmic = COMPAT_MAXIM_MAX77686;
		}
		debug("%s: detected PMIC: %d\n", __func__, local.pmic);
	}
	return local.pmic;
}

static int ft_remove_pmic_node(void *fdt, enum fdt_compat_id id)
{
	int node;

	node = fdtdec_next_compatible(fdt, 0, id);
	if (node < 0)
		return -ENOENT;

	debug("remove %s from kernel DT\n", fdtdec_get_compatible(id));
	return fdt_nop_node(fdt, node);
}

static int ft_board_setup_pmic(void *blob, bd_t *bd)
{
	int pmic = board_get_pmic();

	/* we have at most one PMIC chip : remove the other one */
	switch (pmic) {
	case COMPAT_SAMSUNG_S5M8767:
		ft_remove_pmic_node(blob, COMPAT_MAXIM_MAX77686);
		break;
	case COMPAT_MAXIM_MAX77686:
		ft_remove_pmic_node(blob, COMPAT_SAMSUNG_S5M8767);
		break;
	default:
		debug("No PMIC detected\n");
	}

	return 0;
}

/**
 * Fix-up the kernel device tree so the bridge pd_n and rst_n gpios accurately
 * reflect the current board rev.
 *
 * @param blob		Device tree blob
 * @param bd		Pointer to board information
 * @return 0 if ok, -1 on error (e.g. not enough space in fdt)
 */
static int ft_board_setup_gpios(void *blob, bd_t *bd)
{
	int ret, rev, np, len;
	const struct fdt_property *prop;

	/* Do nothing for newer boards */
	rev = board_get_revision();
	if (rev < 4 || rev == 6)
		return 0;

	/*
	 * If this is an older board, replace powerdown-gpio contents with that
	 * of reset-gpio and delete reset-gpio from the dt.
	 * Also do nothing if we have a Parade PS8622 bridge.
	 */
	np = fdtdec_next_compatible(blob, 0, COMPAT_NXP_PTN3460);
	if (np < 0) {
		debug("%s: Could not find COMPAT_NXP_PTN3460\n", __func__);
		return 0;
	}

	prop = fdt_get_property(blob, np, "reset-gpio", &len);
	if (!prop) {
		debug("%s: Could not get property err=%d\n", __func__, len);
		return -1;
	}

	ret = fdt_setprop_inplace(blob, np, "powerdown-gpio", prop->data,
			len);
	if (ret) {
		debug("%s: Could not setprop inplace err=%d\n", __func__, ret);
		return -1;
	}

	ret = fdt_delprop(blob, np, "reset-gpio");
	if (ret) {
		debug("%s: Could not delprop err=%d\n", __func__, ret);
		return -1;
	}

	return 0;
}

/**
 * Fix-up the kernel device tree so the powered-while-resumed is added to MP
 * device tree.
 *
 * @param blob		Device tree blob
 * @param bd		Pointer to board information
 * @return 0 if ok, -1 on error (e.g. not enough space in fdt)
 */
static int ft_board_setup_tpm_resume(void *blob, bd_t *bd)
{
	const char kernel_tpm_compat[] = "infineon,slb9635tt";
	const char prop_name[] = "powered-while-suspended";
	int err, node, rev;

	/* Only apply fixup to MP machine */
	rev = board_get_revision();
	if (!(rev == 0 || rev == 3))
		return 0;

	node = fdt_node_offset_by_compatible(blob, 0, kernel_tpm_compat);
	if (node < 0) {
		debug("%s: Could not find %s: %d\n", __func__,
				kernel_tpm_compat, node);
		return 0;
	}

	err = fdt_setprop(blob, node, prop_name, NULL, 0);
	if (err) {
		debug("%s: Could not setprop: %d\n", __func__, err);
		return -1;
	}

	return 0;
}

static int ft_board_setup_lcd(void *blob, bd_t *bd)
{
	const char name[] = "simple-framebuffer";
	const char fmt[] = "r5g6b5";
	int reg[2];
	int r;
	int node;

	node = fdt_add_subnode(blob, 0, "framebuffer");
	if (node < 0)
		return -1;
	fdt_setprop(blob, node, "compatible", name, sizeof(name));
	reg[0] = cpu_to_fdt32((unsigned int)lcd_base);
	reg[1] = cpu_to_fdt32(0x00202000);
	fdt_setprop(blob, node, "reg", reg, sizeof(reg));
	r = cpu_to_fdt32(1366);
	fdt_setprop(blob, node, "width", &r, sizeof(r));
	r = cpu_to_fdt32(768);
	fdt_setprop(blob, node, "height", &r, sizeof(r));
	r = cpu_to_fdt32(1366*2);
	fdt_setprop(blob, node, "stride", &r, sizeof(r));
	fdt_setprop(blob, node, "format", fmt, sizeof(fmt));
	return 0;
}

int ft_system_setup(void *blob, bd_t *bd)
{
	if (ft_board_setup_gpios(blob, bd))
		return -1;
	if (ft_board_setup_pmic(blob, bd))
		return -1;
	if (ft_board_setup_lcd(blob, bd))
		return -1;
	return ft_board_setup_tpm_resume(blob, bd);
}

__weak int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_system_setup(blob, bd);
}

#ifdef CONFIG_TPS65090_POWER
/* Switch on a TPSchrome LDO */
int board_ldo_enable(int id)
{
	if (local.ec_passthrough)
		return mkbp_tps65090_fet_enable(id);
	else
		return tps65090_fet_enable(id);
}

/* Switch off a TPSchrome LDO */
int board_ldo_disable(int id)
{
	if (local.ec_passthrough)
		return mkbp_tps65090_fet_disable(id);
	else
		return tps65090_fet_disable(id);
}

int board_dp_lcd_vdd(const void *blob, unsigned *wait_ms)
{
	*wait_ms = 0;
	return board_ldo_enable(6); /* Enable FET6, lcd panel */
}
#endif

static int board_dp_fill_gpios(const void *blob, int *is_ps8622)
{
	int np, ret = 0, rev;

	np = fdtdec_next_compatible(blob, 0, COMPAT_NXP_PTN3460);
	if (np < 0) {
		np = fdtdec_next_compatible(blob, 0, COMPAT_PARADE_PS8622);
		*is_ps8622 = (np < 0) ? 0 : 1;
	}
	if (np < 0) {
		debug("%s: Could not find PTN3460 or PS8622 (%d)\n", __func__,
			ret);
		return np;
	}
	ret = fdtdec_decode_gpio(blob, np, "powerdown-gpio", &local.dp_pd);
	if (ret) {
		debug("%s: Could not decode powerdown-gpio (%d)\n", __func__,
			ret);
		return ret;
	}
	ret = fdtdec_decode_gpio(blob, np, "reset-gpio", &local.dp_rst);
	if (ret) {
		debug("%s: Could not decode reset-gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_decode_gpio(blob, np, "hotplug-gpio", &local.dp_hpd);
	if (ret) {
		debug("%s: Could not decode hotplug (%d)\n", __func__, ret);
		return ret;
	}

	/* Parade is only on newer boards */
	if (*is_ps8622)
		return 0;

	/* If board is older, replace pd gpio with rst gpio */
	rev = board_get_revision();
	if (rev >= 4 && rev != 6) {
		local.dp_pd = local.dp_rst;
		local.dp_rst.gpio = FDT_GPIO_NONE;
	}
	return 0;
}

int board_dp_bridge_setup(const void *blob, unsigned *wait_ms)
{
	int ret;
	int is_ps8622 = 0;

	*wait_ms = 0;

	ret = board_dp_fill_gpios(blob, &is_ps8622);
	if (ret)
		return ret;

	if (is_ps8622) {
		/* set the LDO providing the 1.2V rail to the Parade bridge */
		if (board_get_pmic() == COMPAT_SAMSUNG_S5M8767) {
			ret = s5m8767_volsetting(S5M8767_LDO6,
				CONFIG_VDD_LDO17_MV, S5M8767_REG_ENABLE,
				S5M8767_MV);
		} else {
			ret = max77686_volsetting(MAX77686_LDO17,
				CONFIG_VDD_LDO17_MV, MAX77686_REG_ENABLE,
				MAX77686_MV);
		}
		/* need to wait 20ms after power on before doing I2C writes */
		*wait_ms = 20;
	} else {
		/* Mux HPHPD to the special hotplug detect mode */
		exynos_pinmux_config(PERIPH_ID_DPHPD, 0);
	}

	/* Setup the GPIOs */
	ret = fdtdec_setup_gpio(&local.dp_pd);
	if (ret) {
		debug("%s: Could not setup pd gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_setup_gpio(&local.dp_rst);
	if (ret) {
		debug("%s: Could not setup rst gpio (%d)\n", __func__, ret);
		return ret;
	}
	ret = fdtdec_setup_gpio(&local.dp_hpd);
	if (ret) {
		debug("%s: Could not setup hpd gpio (%d)\n", __func__, ret);
		return ret;
	}

	fdtdec_set_gpio(&local.dp_pd, 0);
	gpio_cfg_pin(local.dp_pd.gpio, EXYNOS_GPIO_OUTPUT);
	gpio_set_pull(local.dp_pd.gpio, EXYNOS_GPIO_PULL_NONE);
	if (fdt_gpio_isvalid(&local.dp_rst)) {
		fdtdec_set_gpio(&local.dp_rst, 1);
		gpio_cfg_pin(local.dp_rst.gpio, EXYNOS_GPIO_OUTPUT);
		gpio_set_pull(local.dp_rst.gpio, EXYNOS_GPIO_PULL_NONE);
		udelay(10);
		fdtdec_set_gpio(&local.dp_rst, 0);
	}

	return 0;
}

/**
 * Write a PS8622 eDP bridge i2c register
 *
 * @param	addr_off	offset from the i2c base address for ps8622
 * @param	reg_addr	register address to write
 * @param	value		value to be written
 * @return	0 on success, non-0 on failure
 */
static int ps8622_i2c_write(unsigned addr_off, unsigned char reg_addr,
			    unsigned char value)
{
	int ret;

	/* TODO: read PS8622 address from FDT */
	ret = i2c_write(0x8 + addr_off, reg_addr, 1, &value, 1);
	debug("%s: reg=%#x, value=%#x, ret=%d\n", __func__, reg_addr, value,
	      ret);
	return ret;
}

static void ps8622_init(void)
{
	i2c_set_bus_num(7);

	debug("PS8622 I2C init\n");
	ps8622_i2c_write(0x02, 0xa1, 0x01); /* HPD low */
	/* SW setting */
	ps8622_i2c_write(0x04, 0x14, 0x01); /* [1:0] SW output 1.2V voltage is
					     * lower to 96% */
	/* RCO SS setting */
	ps8622_i2c_write(0x04, 0xe3, 0x20); /* [5:4] = b01 0.5%, b10 1%, b11
					     * 1.5% */
	ps8622_i2c_write(0x04, 0xe2, 0x80); /* [7] RCO SS enable */
	/* RPHY Setting */
	ps8622_i2c_write(0x04, 0x8a, 0x0c); /* [3:2] CDR tune wait cycle before
					     * measure for fine tune b00: 1us,
					     * 01: 0.5us, 10:2us, 11:4us. */
	ps8622_i2c_write(0x04, 0x89, 0x08); /* [3] RFD always on */
	ps8622_i2c_write(0x04, 0x71, 0x2d); /* CTN lock in/out:
					     * 20000ppm/80000ppm. Lock out 2
					     * times. */
	/* 2.7G CDR settings */
	ps8622_i2c_write(0x04, 0x7d, 0x07); /* NOF=40LSB for HBR CDR setting */
	ps8622_i2c_write(0x04, 0x7b, 0x00); /* [1:0] Fmin=+4bands */
	ps8622_i2c_write(0x04, 0x7a, 0xfd); /* [7:5] DCO_FTRNG=+-40% */
	/* 1.62G CDR settings */
	ps8622_i2c_write(0x04, 0xc0, 0x12); /* [5:2]NOF=64LSB [1:0]DCO scale is
					     * 2/5 */
	ps8622_i2c_write(0x04, 0xc1, 0x92); /* Gitune=-37% */
	ps8622_i2c_write(0x04, 0xc2, 0x1c); /* Fbstep=100% */
	ps8622_i2c_write(0x04, 0x32, 0x80); /* [7] LOS signal disable */
	/* RPIO Setting */
	ps8622_i2c_write(0x04, 0x00, 0xb0); /* [7:4] LVDS driver bias current :
					     * 75% (250mV swing) */
	ps8622_i2c_write(0x04, 0x15, 0x40); /* [7:6] Right-bar GPIO output
					     * strength is 8mA */
	/* EQ Training State Machine Setting */
	ps8622_i2c_write(0x04, 0x54, 0x10); /* RCO calibration start */
	ps8622_i2c_write(0x01, 0x02, 0x81); /* [4:0] MAX_LANE_COUNT set to one
					     * lane */
	ps8622_i2c_write(0x01, 0x21, 0x81); /* [4:0] LANE_COUNT_SET set to one
					     * lane */
	ps8622_i2c_write(0x00, 0x52, 0x20);
	ps8622_i2c_write(0x00, 0xf1, 0x03); /* HPD CP toggle enable */
	ps8622_i2c_write(0x00, 0x62, 0x41);
	ps8622_i2c_write(0x00, 0xf6, 0x01); /* Counter number, add 1ms counter
					     * delay */
	ps8622_i2c_write(0x00, 0x77, 0x06); /* [6]PWM function control by
					     * DPCD0040f[7], default is PWM
					     * block always works. */
	ps8622_i2c_write(0x00, 0x4c, 0x04); /* 04h Adjust VTotal tolerance to
					     * fix the 30Hz no display issue */
	ps8622_i2c_write(0x01, 0xc0, 0x00); /* DPCD00400='h00, Parade OUI =
					     * 'h001cf8 */
	ps8622_i2c_write(0x01, 0xc1, 0x1c); /* DPCD00401='h1c */
	ps8622_i2c_write(0x01, 0xc2, 0xf8); /* DPCD00402='hf8 */
	ps8622_i2c_write(0x01, 0xc3, 0x44); /* DPCD403~408 = ASCII code
					     * D2SLV5='h4432534c5635 */
	ps8622_i2c_write(0x01, 0xc4, 0x32); /* DPCD404 */
	ps8622_i2c_write(0x01, 0xc5, 0x53); /* DPCD405 */
	ps8622_i2c_write(0x01, 0xc6, 0x4c); /* DPCD406 */
	ps8622_i2c_write(0x01, 0xc7, 0x56); /* DPCD407 */
	ps8622_i2c_write(0x01, 0xc8, 0x35); /* DPCD408 */
	ps8622_i2c_write(0x01, 0xca, 0x01); /* DPCD40A, Initial Code major
					     * revision '01' */
	ps8622_i2c_write(0x01, 0xcb, 0x05); /* DPCD40B, Initial Code minor
					     * revision '05' */
	ps8622_i2c_write(0x01, 0xa5, 0xa0); /* DPCD720, Select internal PWM */
	ps8622_i2c_write(0x01, 0xa7, 0xff); /* FFh for 100% PWM of brightness,
					     * 0h for 0% brightness */
	ps8622_i2c_write(0x01, 0xcc, 0x13); /* Set LVDS output as 6bit-VESA
					     * mapping, single LVDS channel */
	ps8622_i2c_write(0x02, 0xb1, 0x20); /* Enable SSC set by register */
	ps8622_i2c_write(0x04, 0x10, 0x16); /* Set SSC enabled and +/-1% central
					     * spreading */
	ps8622_i2c_write(0x04, 0x59, 0x60); /* MPU Clock source: LC => RCO */
	ps8622_i2c_write(0x04, 0x54, 0x14); /* LC -> RCO */
	ps8622_i2c_write(0x02, 0xa1, 0x91); /* HPD high */

	i2c_set_bus_num(0);
}

int board_dp_bridge_init(const void *blob, unsigned *wait_ms)
{
	/* De-assert PD (and possibly RST) to power up the bridge */
	fdtdec_set_gpio(&local.dp_pd, 0);

	/* Ignore the return value here, on some boards this is NC */
	fdtdec_set_gpio(&local.dp_rst, 0);

	/* Configure I2C registers for Parade bridge */
	if (fdtdec_next_compatible(blob, 0, COMPAT_PARADE_PS8622) >= 0)
		ps8622_init();

	/*
	 * We need to wait for 90ms after bringing up the bridge since there
	 * is a phantom "high" on the HPD chip during its bootup.  The phantom
	 * high comes within 7ms of de-asserting PD and persists for at least
	 * 15ms.  The real high comes roughly 50ms after PD is de-asserted. The
	 * phantom high makes it hard for us to know when the NXP chip is up.
	 */
	*wait_ms = 90;
	return 0;
}

int board_dp_bridge_reset(const void *blob, unsigned *wait_ms)
{
	debug("%s: eDP bridge failed to come up\n", __func__);

	/*
	 * If we're here, the bridge chip failed to initialize.
	 * Drive DP_N low in an attempt to reset.
	 */
	fdtdec_set_gpio(&local.dp_pd, 1);

	/* Ignore the return value here, on some boards this is NC */
	fdtdec_set_gpio(&local.dp_rst, 1);

	/*
	 * Arbitrarily wait 300ms here with DP_N low.  Don't know for
	 * sure how long we should wait, but we're being paranoid.
	 */
	*wait_ms = 300;
	return 0;
}

int board_dp_hotplug(const void *blob, unsigned *wait_ms)
{
	const int MAX_TRIES = 10;
	static int num_tries;

	/* Check HPD.  If it's high, we're all good. */
	if (fdtdec_get_gpio(&local.dp_hpd)) {
		*wait_ms = 0;
		return 0;
	}

	debug("%s: eDP bridge failed to come up; try %d of %d\n", __func__,
		num_tries, MAX_TRIES);
	/* Immediately go into bridge reset if the hp line is not high */
	*wait_ms = 0;
	++num_tries;
	return num_tries <= MAX_TRIES ? -EAGAIN : -ENODEV;
}

#ifdef CONFIG_TPS65090_POWER
int board_dp_backlight_vdd(const void *blob, unsigned *wait_ms)
{
	/* This delay is T5 in the LCD timing spec (defined as > 10ms) */
	*wait_ms = 10;
	return board_ldo_enable(1); /* Enable FET1, backlight */
}
#endif

int board_dp_backlight_pwm(const void *blob, unsigned *wait_ms)
{
	/*
	 * Configure backlight PWM as a simple output high (100% brightness)
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_direction_output(GPIO_B20, 1);
	/* This delay is T6 in the LCD timing spec (defined as > 10ms) */
	*wait_ms = 10;
	return 0;
}

int board_dp_backlight_en(const void *blob, unsigned *wait_ms)
{
	/*
	 * Configure GPIO for LCD_BL_EN
	 * TODO(hatim.rv@samsung.com): Move to FDT
	 */
	gpio_direction_output(GPIO_X30, 1);
	/* We're done, no more delays! */
	*wait_ms = 0;
	return 0;
}

static void board_enable_audio_codec(void)
{
	int node, ret, value;
	struct fdt_gpio_state en_gpio;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_SAMSUNG_EXYNOS_SOUND);
	if (node <= 0)
		return;

	ret = fdtdec_decode_gpio(gd->fdt_blob, node, "codec-enable-gpio",
				&en_gpio);
	if (ret == -FDT_ERR_NOTFOUND)
		return;

	/* Turn on the GPIO which connects to the codec's "enable" line. */
	value = (en_gpio.flags & FDT_GPIO_ACTIVE_LOW) ? 0 : 1;
	gpio_direction_output(en_gpio.gpio, value);
	gpio_set_pull(en_gpio.gpio, EXYNOS_GPIO_PULL_NONE);
}

static void board_configure_analogix(void)
{
	int node, ret;
	struct fdt_gpio_state reset_gpio;
	struct fdt_gpio_state powerdown_gpio;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_ANALOGIX_ANX7805);
	if (node <= 0)
		return;

	/* Configure Analogix 780x bridge in USB pass-through mode */
	ret = fdtdec_decode_gpio(gd->fdt_blob, node, "reset-gpio",
				&reset_gpio);
	if (ret < 0) {
		debug("%s: Could not find reset-gpio", __func__);
		return;
	}

	ret = fdtdec_decode_gpio(gd->fdt_blob, node, "powerdown-gpio",
				&powerdown_gpio);
	if (ret < 0) {
		debug("%s: Could not find powerdown-gpio", __func__);
		return;
	}

	fdtdec_setup_gpio(&powerdown_gpio);
	fdtdec_setup_gpio(&reset_gpio);
	gpio_direction_output(powerdown_gpio.gpio, 1);
	gpio_direction_output(reset_gpio.gpio, 1);
}

int board_init(void)
{
	struct fdt_memory mem_config;
	int ret;

	/* Record the time we spent before SPL */
	bootstage_add_record(BOOTSTAGE_ID_START_SPL, "spl_start", 0,
			     CONFIG_SPL_TIME_US);
	bootstage_mark_name(BOOTSTAGE_ID_BOARD_INIT, "board_init");

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

	if (board_init_mkbp_devices(gd->fdt_blob))
		return -1;

#ifdef CONFIG_TPS65090_POWER
	if (mkbp_tps65090_init() < 0) {
		local.ec_passthrough = 0;
		tps65090_init();
	} else {
		local.ec_passthrough = 1;
	}

	/*
	 * If we just reset, disable the backlight and lcd fets before
	 * [re-]initializing the lcd. This ensures we are always in the same
	 * state during lcd init. We've seen some oddities with these fets, so
	 * this removes a bit of uncertainty.
	 */
	if (board_is_processor_reset()) {
		board_ldo_disable(1);
		board_ldo_disable(6);
	}
#endif
	exynos_lcd_check_next_stage(gd->fdt_blob, 0);

	if (board_get_pmic() == COMPAT_SAMSUNG_S5M8767)
		ret = s5m8767_enable_32khz_cp();
	else
		ret = max77686_enable_32khz_cp();
	if (ret) {
		debug("%s: Failed to enable 32khz coprocessor clock\n",
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

	/* Clock Gating all the unused IP's to save power */
	clock_gate();

	/* Disable USB3.0 PLL to save 250mW of power */
	disable_usb30_pll();

	board_configure_analogix();

	board_enable_audio_codec();

	exynos_lcd_check_next_stage(gd->fdt_blob, 0);

	bootstage_mark_name(BOOTSTAGE_ID_BOARD_INIT_DONE, "board_init_done");

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
		while (get_timer(start_retry) < local.wait_retry_ms) {
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

		mdelay(local.wait_retry_ms);
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

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	stdio_print_current_devices();

	if (local.mkbp_err) {
		/* Force console and LCD display on */
		gd->flags &= ~GD_FLG_SILENT;
		exynos_lcd_check_next_stage(gd->fdt_blob, 1);

		printf("Chrome EC communications failure %d\n",
		       local.mkbp_err);
		puts("\nPlease reset with Power+Refresh\n\n");
		hang();
		/*
		 * Replace hang() with panic() once debugged in test lab.
		 * panic("Cannot init mkbp device");
		 */
		return -1;
	}
	return 0;
}
#endif
