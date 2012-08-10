/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
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

#include <asm/arch/pinmux.h>
#include <malloc.h>
#include <common.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <i2c.h>
#include <sound.h>
#include "i2s.h"
#include "wm8994.h"
#include "max98095.h"

/* defines */
#define SOUND_400_HZ 400
#define SOUND_BITS_IN_BYTE 8

/* pcm values for 400hz 48k sampling rate from 1 cycle */
unsigned short sine_table_400[120] = {
	0, 1714, 3425, 5125, 6812, 8480, 10125, 11742, 13327, 14875, 16383,
	17846, 19259, 20620, 21925, 23169, 24350, 25464, 26509, 27480, 28377,
	29195, 29934, 30590, 31163, 31650, 32050, 32363, 32587, 32722, 32767,
	32722, 32587, 32363, 32050, 31650, 31163, 30590, 29934, 29195, 28377,
	27480, 26509, 25464, 24350, 23169, 21925, 20620, 19259, 17846, 16383,
	14875, 13327, 11742, 10125, 8480, 6812, 5125, 3425, 1714, 0, -1714,
	-3425, -5125, -6812, -8480, -10125, -11742, -13327, -14875, -16383,
	-17846, -19259, -20620, -21925, -23169, -24350, -25464, -26509,
	-27480, -28377, -29195, -29934, -30590, -31163, -31650, -32050,
	-32363, -32587, -32722, -32767, -32722, -32587, -32363, -32050,
	-31650, -31163, -30590, -29934, -29195, -28377, -27480, -26509,
	-25464, -24350, -23169, -21925, -20620, -19259, -17846, -16383,
	-14875, -13327, -11742, -10125, -8480, -6812, -5125, -3425, -1714
};

/* Globals */
struct i2stx_info g_i2stx_pri;
struct sound_codec_info g_codec_info;

/*
 * get_sound_fdt_values gets fdt values for i2s parameters
 *
 * @param i2stx_info	i2s transmitter transfer param structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_sound_i2s_fdt_values(struct i2stx_info *i2s, const void *blob)
{
	int node;
	int error = 0;
	int base;

	/* Get the node from FDT for sound */
	node = fdtdec_next_compatible(blob, 0,
					COMPAT_SAMSUNG_EXYNOS_SOUND);
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for sound in device tree\n");
		return -1;
	}

	/*
	 * Get the pre-defined sound specific values from FDT.
	 * All of these are expected to be correct otherwise
	 * wrong register values in i2s setup parameters
	 * may result in no sound play.
	 */
	base = fdtdec_get_addr(blob, node, "reg");
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Missing  i2s base\n", __func__);
		return -1;
	}

	i2s->base_address = base;
	i2s->audio_pll_clk = fdtdec_get_int(blob,
				node, "samsung,i2s-epll-clock-frequency", -1);
	error |= i2s->audio_pll_clk;
	debug("audio_pll_clk = %d\n", i2s->audio_pll_clk);
	i2s->samplingrate = fdtdec_get_int(blob,
				node, "samsung,i2s-sampling-rate", -1);
	error |= i2s->samplingrate;
	debug("samplingrate = %d\n", i2s->samplingrate);
	i2s->bitspersample = fdtdec_get_int(blob,
				node, "samsung,i2s-bits-per-sample", -1);
	error |= i2s->bitspersample;
	debug("bitspersample = %d\n", i2s->bitspersample);
	i2s->channels = fdtdec_get_int(blob,
				node, "samsung,i2s-channels", -1);
	error |= i2s->channels;
	debug("channels = %d\n", i2s->channels);
	i2s->rfs = fdtdec_get_int(blob,
				node, "samsung,i2s-lr-clk-framesize", -1);
	error |= i2s->rfs;
	debug("rfs = %d\n", i2s->rfs);
	i2s->bfs = fdtdec_get_int(blob,
				node, "samsung,i2s-bit-clk-framesize", -1);
	error |= i2s->bfs;
	debug("bfs = %d\n", i2s->bfs);
	if (error == -1) {
		debug("fail to get sound i2s node properties\n");
		return -1;
	}

	return 0;
}

/*
 * Gets fdt values for wm8994 config parameters
 *
 * @param pcodec_info	codec information structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_sound_fdt_values(struct sound_codec_info *pcodec_info,
				const void *blob, enum fdt_compat_id compat_id)
{
	enum fdt_compat_id compat;
	int node;
	int error = 0;
	int parent;

	/* Get the node from FDT for codec */
	node = fdtdec_next_compatible(blob, 0, compat_id);
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for codec in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	compat = fdtdec_lookup(blob, parent);
	switch (compat) {
	case COMPAT_SAMSUNG_EXYNOS_SPI:
		debug("%s: Support not added for SPI interface\n", __func__);
		return -1;
		break;
	case COMPAT_SAMSUNG_S3C2440_I2C:
		pcodec_info->i2c_bus = i2c_get_bus_num_fdt(blob, parent);
		error |= pcodec_info->i2c_bus;
		debug("i2c bus = %d\n", pcodec_info->i2c_bus);
		pcodec_info->i2c_dev_addr = fdtdec_get_int(blob, node, "reg", 0);
		error |= pcodec_info->i2c_dev_addr;
		debug("i2c dev addr = %d\n", pcodec_info->i2c_dev_addr);
		break;
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}

	if (error == -1) {
		debug("fail to get codec node properties\n");
		return -1;
	}

	return 0;
}

/*
 * Gets fdt values for codec config parameters
 *
 * @param pcodec_info	codec information structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_sound_codec_fdt_values(struct sound_codec_info *pcodec_info,
							const void *blob)
{
	int node;
	int error = 0;
	const char *codectype;

	/* Get the node from FDT for sound */
	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS_SOUND);
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for sound in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	/*
	 * Get the pre-defined sound codec specific values from FDT.
	 * All of these are expected to be correct otherwise sound
	 * can not be played
	 */
	codectype = fdt_getprop(blob, node, "samsung,codec-type", NULL);
	debug("device = %s\n", codectype);

	if (!strcmp(codectype, "wm8994")) {
		pcodec_info->codec_type = CODEC_WM_8994;
		error = get_sound_fdt_values(pcodec_info, blob,
					     COMPAT_WOLFSON_WM8994_CODEC);
	} else if (!strcmp(codectype, "max98095")) {
		pcodec_info->codec_type = CODEC_MAX_98095;
		error = get_sound_fdt_values(pcodec_info, blob,
					     COMPAT_MAXIM_98095_CODEC);
	} else
		error = -1;

	if (error == -1) {
		debug("fail to get sound codec node properties\n");
		return -1;
	}

	return 0;
}

int sound_init(const void *blob)
{
	int ret;
	struct i2stx_info *pi2s_tx = &g_i2stx_pri;
	struct sound_codec_info *pcodec_info = &g_codec_info;

	/* Get the I2S Values */
	if (get_sound_i2s_fdt_values(pi2s_tx, blob) < 0)
		return -1;

	/* Get the codec Values */
	if (get_sound_codec_fdt_values(pcodec_info, blob) < 0)
		return -1;

	ret = i2s_tx_init(pi2s_tx);
	if (ret) {
		debug("%s: Failed to init i2c transmit: ret=%d\n", __func__,
		      ret);
		return ret;
	}

	/* Check the codec type and initialise the same */
	if (pcodec_info->codec_type == CODEC_WM_8994) {
		ret = wm8994_init(pcodec_info, WM8994_AIF2,
			pi2s_tx->samplingrate,
			(pi2s_tx->samplingrate * (pi2s_tx->rfs)),
			pi2s_tx->bitspersample, pi2s_tx->channels);
	} else if (pcodec_info->codec_type == CODEC_MAX_98095) {
#if defined CONFIG_SOUND_MAX98095
		ret = max98095_init(pcodec_info, pi2s_tx->samplingrate,
			(pi2s_tx->samplingrate * (pi2s_tx->rfs)),
			pi2s_tx->bitspersample);
#else
		ret = -1;
		debug("%s: max98095 codec support not built in.\n", __func__);
#endif
	} else {
		debug("%s: Unknown code type %d\n", __func__,
		      pcodec_info->codec_type);
		return -1;
	}
	if (ret) {
		debug("%s: Codec init failed\n", __func__);
		return -1;
	}

	return ret;
}

/*
 * Generates 400hz sine wave data for 1sec
 *
 * @param samplingrate	samplinng rate of the sinewave need to be generated
 * @param data		data buffer pointer
 */
static void sound_prepare_sinewave_400hz_buffer(unsigned short *data)
{
	int freq = SOUND_400_HZ;
	int i;

	while (freq--) {
		i = ARRAY_SIZE(sine_table_400);

		for (i = 0; i < ARRAY_SIZE(sine_table_400); i++) {
			*data++ = sine_table_400[i];
			*data++ = sine_table_400[i];
		}
	}
}

int sound_play(void)
{
	unsigned int *data;
	unsigned long data_size;
	unsigned int ret;

	/* Sine wave Buffer length computation */
	data_size = g_i2stx_pri.samplingrate * g_i2stx_pri.channels;
	data_size *= (g_i2stx_pri.bitspersample / SOUND_BITS_IN_BYTE);
	data = malloc(data_size);

	if (data == NULL ) {
		debug("%s: malloc failed\n", __func__);
		return -1;
	}

	sound_prepare_sinewave_400hz_buffer((unsigned short *)data);

	ret = i2s_transfer_tx_data(&g_i2stx_pri, data,
				   (data_size / sizeof(int)));
	free(data);

	return ret;
}
