/*
 * Chromium OS Matrix Keyboard
 *
 * Copyright (c) 2012 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <fdtdec.h>
#include <input.h>
#include <key_matrix.h>
#include <mkbp.h>
#include <stdio_dev.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	KBC_MAX_KEYS		= 8,	/* Maximum keys held down at once */
};

static struct keyb {
	struct mkbp_dev *dev;		/* The MKBP device */
	struct input_config input;	/* The input layer */
	struct key_matrix matrix;	/* The key matrix layer */
	int key_rows;			/* Number of keyboard rows */
	int key_cols;			/* Number of keyboard columns */
	unsigned int repeat_delay_ms;	/* Time before autorepeat starts */
	unsigned int repeat_rate_ms;	/* Autorepeat rate in ms */
	int ghost_filter;		/* 1 to enable ghost filter, else 0 */
	int inited;			/* 1 if keyboard is ready */
} config;


/**
 * Check the keyboard controller and return a list of key matrix positions
 * for which a key is pressed
 *
 * @param config	Keyboard config
 * @param keys		List of keys that we have detected
 * @param max_count	Maximum number of keys to return
 * @return number of pressed keys, 0 for none
 */
static int check_for_keys(struct keyb *config,
			   struct key_matrix_key *keys, int max_count)
{
	struct key_matrix_key *key;
	struct mbkp_keyscan scan;
	unsigned int row, col, bit, data;
	int num_keys;

	if (mkbp_scan_keyboard(config->dev, &scan)) {
		debug("%s: keyboard scan failed\n", __func__);
		return -1;
	}

	/* TODO(sjg@chromium,org): Should perhaps optimize this algorithm */
	for (col = num_keys = bit = 0; col < config->matrix.num_cols;
			col++) {
		for (row = 0; row < config->matrix.num_rows; row++) {
			unsigned int mask = 1 << (bit & 7);

			data = scan.data[bit / 8];
			if ((data & mask) && num_keys < max_count) {
				key = keys + num_keys++;
				key->row = row;
				key->col = col;
				key->valid = 1;
			}
			bit++;
		}
	}

	return num_keys;
}

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
static int kbd_tstc(void)
{
	if (mkbp_interrupt_pending(config.dev)) {
		/* Just get input to do this for us */
		return input_tstc(&config.input);
	}

	return 0;
}

/**
 * Read a key
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
static int kbd_getc(void)
{
	/* Just get input to do this for us */
	return config.inited ? input_getc(&config.input) : 0;
}

/**
 * Check the keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * @return 1, to indicate that we have something to look at
 */
int mkbp_kbc_check(struct input_config *input)
{
	struct key_matrix_key keys[KBC_MAX_KEYS];
	int keycodes[KBC_MAX_KEYS];
	int num_keys, num_keycodes;

	num_keys = check_for_keys(&config, keys, KBC_MAX_KEYS);
	if (num_keys < 0)
		return -1;
	num_keycodes = key_matrix_decode(&config.matrix, keys, num_keys,
					 keycodes, KBC_MAX_KEYS);
	input_send_keycodes(input, keycodes, num_keycodes);

	return 1;
}

/**
 * Decode MBKP keyboard details from the device tree
 *
 * @param blob		Device tree blob
 * @param node		Node to decode from
 * @param config	Configuration data read from fdt
 * @return 0 if ok, -1 on error
 */
static int mkbp_keyb_decode_fdt(const void *blob, int node,
				struct keyb *config)
{
	/*
	 * Get keyboard rows and columns - at present we are limited to
	 * 8 columns by the protocol (one byte per row scan)
	 */
	config->key_rows = fdtdec_get_int(blob, node, "google,key-rows", 0);
	config->key_cols = fdtdec_get_int(blob, node, "google,key-columns", 0);
	if (!config->key_rows || !config->key_cols ||
			config->key_rows * config->key_cols / 8
				> MKBP_KEYSCAN_COLS) {
		debug("%s: Invalid key matrix size %d x %d\n", __func__,
		      config->key_rows, config->key_cols);
		return -1;
	}
	config->repeat_delay_ms = fdtdec_get_int(blob, node,
						 "google,repeat-delay-ms", 0);
	config->repeat_rate_ms = fdtdec_get_int(blob, node,
						"google,repeat-rate-ms", 0);
	config->ghost_filter = fdtdec_get_bool(blob, node,
					       "google,ghost-filter");
	return 0;
}

/**
 * Set up the keyboard. This is called by the stdio device handler.
 *
 * We want to do this init when the keyboard is actually used rather than
 * at start-up, since keyboard input may not currently be selected.
 *
 * @return 0 if ok, -1 on error
 */
static int mkbp_init_keyboard(void)
{
	const void *blob = gd->fdt_blob;
	int node;

	config.dev = board_get_mkbp_dev();
	if (!config.dev) {
		debug("%s: no mkbp device: cannot init keyboard\n", __func__);
		return -1;
	}
	node = fdtdec_next_compatible(blob, 0, COMPAT_GOOGLE_MKBP_KEYB);
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -1;
	}
	if (mkbp_keyb_decode_fdt(blob, node, &config))
		return -1;
	input_set_delays(&config.input, config.repeat_delay_ms,
			 config.repeat_rate_ms);
	if (key_matrix_init(&config.matrix, config.key_rows,
			config.key_cols, config.ghost_filter)) {
		debug("%s: cannot init key matrix\n", __func__);
		return -1;
	}
	if (key_matrix_decode_fdt(&config.matrix, gd->fdt_blob, node)) {
		debug("%s: Could not decode key matrix from fdt\n", __func__);
		return -1;
	}
	config.inited = 1;
	debug("%s: Matrix keyboard %dx%d ready\n", __func__, config.key_rows,
	      config.key_cols);

	return 0;
}

int drv_keyboard_init(void)
{
	struct stdio_dev dev;

	if (input_init(&config.input, 0)) {
		debug("%s: Cannot set up input\n", __func__);
		return -1;
	}
	config.input.read_keys = mkbp_kbc_check;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "mkbp-keyb");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = kbd_getc;
	dev.tstc = kbd_tstc;
	dev.start = mkbp_init_keyboard;

	/* Register the device. mkbp_init_keyboard() will be called soon */
	return input_stdio_register(&dev);
}
