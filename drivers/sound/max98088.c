/*
 * max98088.c -- MAX98088 ALSA SoC Audio driver
 *
 * Copyright 2010 Maxim Integrated Products
 *
 * Modified for uboot by Chih-Chung Chang (chihchung@chromium.org),
 * following the changes made in max98095.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <common.h>
#include <i2c.h>
#include <sound.h>
#include "i2s.h"
#include "max98088.h"

DECLARE_GLOBAL_DATA_PTR;

enum max98088_type {
	MAX98088,
};

struct max98088_priv {
	enum max98088_type devtype;
	unsigned int sysclk;
	unsigned int rate;
	unsigned int fmt;
};

struct max98088_priv g_max98088_info;
unsigned int g_max98088_i2c_dev_addr;

/* codec mclk clock divider coefficients. Index 0 is reserved. */
static const int rate_table[] = {0, 8000, 11025, 16000, 22050, 24000, 32000,
				 44100, 48000, 88200, 96000};

/*
 * Writes value to a device register through i2c
 *
 * @param reg	reg number to be write
 * @param data	data to be writen to the above registor
 *
 * @return	int value 1 for change, 0 for no change or negative error code.
 */
static int max98088_i2c_write(unsigned int reg, unsigned char data)
{
	debug("%s: Write Addr : 0x%02X, Data :  0x%02X\n",
	      __func__, reg, data);
	return i2c_write(g_max98088_i2c_dev_addr, reg, 1, &data, 1);
}

/*
 * Read a value from a device register through i2c
 *
 * @param reg	reg number to be read
 * @param data	address of read data to be stored
 *
 * @return	int value 0 for success, -1 in case of error.
 */
static unsigned int max98088_i2c_read(unsigned int reg, unsigned char *data)
{
	int ret;

	ret = i2c_read(g_max98088_i2c_dev_addr, reg, 1, data, 1);
	if (ret != 0) {
		debug("%s: Error while reading register %#04x\n",
		      __func__, reg);
		return -1;
	}

	return 0;
}

/*
 * update device register bits through i2c
 *
 * @param reg	codec register
 * @param mask	register mask
 * @param value	new value
 *
 * @return int value 0 for success, non-zero error code.
 */
static int max98088_update_bits(unsigned int reg, unsigned char mask,
				unsigned char value)
{
	int change, ret = 0;
	unsigned char old, new;

	if (max98088_i2c_read(reg, &old) != 0)
		return -1;
	new = (old & ~mask) | (value & mask);
	change  = (old != new) ? 1 : 0;
	if (change)
		ret = max98088_i2c_write(reg, new);
	if (ret < 0)
		return ret;

	return change;
}

/*
 * codec mclk clock divider coefficients based on sampling rate
 *
 * @param rate sampling rate
 * @param value address of indexvalue to be stored
 *
 * @return	0 for success or negative error code.
 */
static int rate_value(int rate, u8 *value)
{
	int i;

	for (i = 1; i < ARRAY_SIZE(rate_table); i++) {
		if (rate_table[i] >= rate) {
			*value = i;
			return 0;
		}
	}
	*value = 1;

	return -1;
}

/*
 * Sets hw params for max98088
 *
 * @param max98088	max98088 information pointer
 * @param rate		Sampling rate
 * @param bits_per_sample	Bits per sample
 *
 * @return -1 for error  and 0  Success.
 */
static int max98088_dai1_hw_params(struct max98088_priv *max98088,
				   unsigned int rate,
				   unsigned int bits_per_sample)
{
	u8 regval;
	int error;

	switch (bits_per_sample) {
	case 16:
		error = max98088_update_bits(M98088_REG_14_DAI1_FORMAT,
					     M98088_DAI_WS, 0);
		break;
	case 24:
		error = max98088_update_bits(M98088_REG_14_DAI1_FORMAT,
					     M98088_DAI_WS, M98088_DAI_WS);
		break;
	default:
		debug("%s: Illegal bits per sample %d.\n",
		      __func__, bits_per_sample);
		return -1;
	}

	error |= max98088_update_bits(M98088_REG_51_PWR_SYS, M98088_SHDNRUN, 0);

	if (rate_value(rate, &regval)) {
		debug("%s: Failed to set sample rate to %d.\n",
		      __func__, rate);
		return -1;
	}

	error |= max98088_update_bits(M98088_REG_11_DAI1_CLKMODE,
				      M98088_CLKMODE_MASK, regval << 4);
	max98088->rate = rate;

	/* Update sample rate mode */
	if (rate < 50000)
		error |= max98088_update_bits(M98088_REG_18_DAI1_FILTERS,
					      M98088_DAI_DHF, 0);
	else
		error |= max98088_update_bits(M98088_REG_18_DAI1_FILTERS,
					      M98088_DAI_DHF, M98088_DAI_DHF);

	error |= max98088_update_bits(M98088_REG_51_PWR_SYS,
				      M98088_SHDNRUN, M98088_SHDNRUN);

	if (error < 0) {
		debug("%s: Error setting hardware params.\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface system clock for the given frequency
 *
 * @param max98088	max98088 information
 * @param freq		Sampling frequency in Hz
 *
 * @return -1 for error and 0 success.
 */
static int max98088_dai_set_sysclk(struct max98088_priv *max98088,
				   unsigned int freq)
{
	unsigned char pwr;
	int error = 0;

	/* Requested clock frequency is already setup */
	if (freq == max98088->sysclk)
		return 0;

	/* Setup clocks for slave mode, and using the PLL
	 * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
	 *         0x02 (when master clk is 20MHz to 30MHz)..
	 */
	if ((freq >= 10000000) && (freq < 20000000)) {
		error = max98088_i2c_write(M98088_REG_10_SYS_CLK, 0x10);
	} else if ((freq >= 20000000) && (freq < 30000000)) {
		error = max98088_i2c_write(M98088_REG_10_SYS_CLK, 0x20);
	} else {
		debug("%s: Invalid master clock frequency\n", __func__);
		return -1;
	}

	error |= max98088_i2c_read(M98088_REG_51_PWR_SYS, &pwr);
	if (pwr & M98088_SHDNRUN) {
		error |= max98088_update_bits(M98088_REG_51_PWR_SYS,
					      M98088_SHDNRUN, 0);
		error |= max98088_update_bits(M98088_REG_51_PWR_SYS,
					      M98088_SHDNRUN, M98088_SHDNRUN);
	}

	debug("Clock source is at %uHz\n", freq);

	if (error < 0)
		return -1;

	max98088->sysclk = freq;
	return 0;
}

/*
 * Sets Max98088 I2S format
 *
 * @param max98088	max98088 information
 * @param fmt		i2S format - supports a subset of the options defined
 *			in i2s.h.
 *
 * @return -1 for error and 0  Success.
 */
static int max98088_dai1_set_fmt(struct max98088_priv *max98088, int fmt)
{
	u8 reg15val;
	u8 reg14val = 0;
	int error = 0;

	if (fmt == max98088->fmt)
		return 0;

	max98088->fmt = fmt;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* Slave mode PLL */
		error |= max98088_i2c_write(M98088_REG_12_DAI1_CLKCFG_HI,
					    0x80);
		error |= max98088_i2c_write(M98088_REG_13_DAI1_CLKCFG_LO,
					    0x00);
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		/* Set to master mode */
		reg14val |= M98088_DAI_MAS;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		debug("%s: Clock mode unsupported\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		reg14val |= M98088_DAI_DLY;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		break;
	default:
		debug("%s: Unrecognized format.\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		reg14val |= M98088_DAI_WCI;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		reg14val |= M98088_DAI_BCI;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		reg14val |= M98088_DAI_BCI|M98088_DAI_WCI;
		break;
	default:
		debug("%s: Unrecognized inversion settings.\n",  __func__);
		return -1;
	}

	error |= max98088_update_bits(M98088_REG_14_DAI1_FORMAT,
				      M98088_DAI_MAS | M98088_DAI_DLY |
				      M98088_DAI_BCI | M98088_DAI_WCI,
				      reg14val);

	reg15val = M98088_DAI_BSEL64;
	error |= max98088_i2c_write(M98088_REG_15_DAI1_CLOCK, reg15val);

	if (error < 0) {
		debug("%s: Error setting i2s format.\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * resets the audio codec
 *
 * @return -1 for error and 0 success.
 */
static int max98088_reset(void)
{
	int i, ret;
	u8 val;

	/*
	 * Reset to hardware default for registers, as there is not a soft
	 * reset hardware control register.
	 */
	for (i = M98088_REG_0F_IRQ_ENABLE; i <= M98088_REG_51_PWR_SYS; i++) {
		switch (i) {
		case M98088_REG_4E_BIAS_CNTL:
			val = 0xf0;
			break;
		case M98088_REG_50_DAC_BIAS2:
			val = 0x0f;
			break;
		default:
			val = 0;
		}
		ret = max98088_i2c_write(i, val);
		if (ret < 0) {
			debug("%s: Failed to reset: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

/*
 * Intialise max98088 codec device
 *
 * @param max98088	max98088 information
 *
 * @returns -1 for error  and 0 Success.
 */
static int max98088_device_init(struct max98088_priv *max98088)
{
	unsigned char id;
	int error = 0;

	/* reset the codec registers to default values */
	error = max98088_reset();
	if (error != 0)
		return error;

	/* initialize private data */
	max98088->sysclk = -1U;
	max98088->rate = -1U;
	max98088->fmt = -1U;

	error = max98088_i2c_read(M98088_REG_FF_REV_ID, &id);
	if (error < 0) {
		debug("%s: Failure reading hardware revision: %d\n",
		      __func__, error);
		goto err_access;
	}
	debug("%s: Hardware revision: %c\n", __func__, (id - 0x40) + 'A');

	error |= max98088_i2c_write(M98088_REG_51_PWR_SYS, M98088_PWRSV);
	error |= max98088_i2c_write(M98088_REG_0F_IRQ_ENABLE, 0x00);

	/*
	 * initialize registers to hardware default configuring audio
	 * interface2 to DAI1
	 */

	error |= max98088_i2c_write(M98088_REG_22_MIX_DAC,
				    M98088_DAI1L_TO_DACL|M98088_DAI1R_TO_DACR);
	error |= max98088_i2c_write(M98088_REG_4E_BIAS_CNTL, 0xF0);
	error |= max98088_i2c_write(M98088_REG_50_DAC_BIAS2, 0x0F);
	error |= max98088_i2c_write(M98088_REG_16_DAI1_IOCFG,
				    M98088_S2NORMAL|M98088_SDATA);

	/* route DACL and DACR output to Speaker and Headphone */
	error |= max98088_i2c_write(M98088_REG_2B_MIX_SPK_LEFT, 1);  /* DACL */
	error |= max98088_i2c_write(M98088_REG_2C_MIX_SPK_RIGHT, 1); /* DACR */
	error |= max98088_i2c_write(M98088_REG_25_MIX_HP_LEFT, 1);   /* DACL */
	error |= max98088_i2c_write(M98088_REG_26_MIX_HP_RIGHT, 1);  /* DACR */

	/* set volume */
	error |= max98088_i2c_write(M98088_REG_3D_LVL_SPK_L, 0x0f); /* -12 dB */
	error |= max98088_i2c_write(M98088_REG_3E_LVL_SPK_R, 0x0f); /* -12 dB */
	error |= max98088_i2c_write(M98088_REG_39_LVL_HP_L, 0x0d);  /* -22 dB */
	error |= max98088_i2c_write(M98088_REG_3A_LVL_HP_R, 0x0d);  /* -22 dB */

	/* power enable */
	error |= max98088_i2c_write(M98088_REG_4D_PWR_EN_OUT,
				    M98088_HPLEN | M98088_HPREN |
				    M98088_SPLEN | M98088_SPREN |
				    M98088_DALEN | M98088_DAREN);

err_access:
	if (error < 0)
		return -1;

	return 0;
}

static int max98088_do_init(struct sound_codec_info *pcodec_info,
			    int sampling_rate, int mclk_freq,
			    int bits_per_sample)
{
	int ret;

	/* Enable codec clock */
	power_enable_xclkout();

	/* shift the device address by 1 for 7 bit addressing */
	g_max98088_i2c_dev_addr = pcodec_info->i2c_dev_addr >> 1;

	if (pcodec_info->codec_type == CODEC_MAX_98088)
		g_max98088_info.devtype = MAX98088;
	else {
		debug("%s: Codec id [%d] not defined\n", __func__,
		      pcodec_info->codec_type);
		return -1;
	}

	ret = max98088_device_init(&g_max98088_info);
	if (ret < 0) {
		debug("%s: max98088 codec chip init failed\n", __func__);
		return ret;
	}

	ret = max98088_dai_set_sysclk(&g_max98088_info, mclk_freq);
	if (ret < 0) {
		debug("%s: max98088 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = max98088_dai1_hw_params(&g_max98088_info, sampling_rate,
				      bits_per_sample);

	if (ret == 0) {
		ret = max98088_dai1_set_fmt(&g_max98088_info,
					    SND_SOC_DAIFMT_I2S |
					    SND_SOC_DAIFMT_NB_NF |
					    SND_SOC_DAIFMT_CBS_CFS);
	}

	return ret;
}

/* max98088 Device Initialisation */
int max98088_init(struct sound_codec_info *pcodec_info,
		  int sampling_rate, int mclk_freq,
		  int bits_per_sample)
{
	int ret;
	int old_bus = i2c_get_bus_num();

	i2c_set_bus_num(pcodec_info->i2c_bus);
	ret = max98088_do_init(pcodec_info, sampling_rate, mclk_freq,
			       bits_per_sample);
	i2c_set_bus_num(old_bus);

	return ret;
}
