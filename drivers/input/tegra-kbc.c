/*
 *  (C) Copyright 2011
 *  NVIDIA Corporation <www.nvidia.com>
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
#include <malloc.h>
#include <linux/input.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/timer.h>
#include <tegra-kbc.h>

#include <fdtdec.h>
#include <input.h>
#include <stdio_dev.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	KBC_MAX_ROW		= 16,
	KBC_MAX_COL		= 8,
	KBC_KEY_COUNT		= KBC_MAX_ROW * KBC_MAX_COL,
	KBC_MAX_GPIO		= 24,
	KBC_MAX_KPENT		= 8,	/* size of keypress entry queue */
};

/* KBC row scan time and delay for beginning the row scan (in usecs) */
#define KBC_ROW_SCAN_TIME	16
#define KBC_ROW_SCAN_DLY	5

/*  uses a 32KHz clock so a cycle = 1/32Khz */
#define KBC_CYCLE_IN_USEC	DIV_ROUND_UP(1000, 32)

#define KBC_FIFO_TH_CNT_SHIFT(cnt)	(cnt << 14)
#define KBC_DEBOUNCE_CNT_SHIFT(cnt)	(cnt << 4)
#define KBC_CONTROL_FIFO_CNT_INT_EN	(1 << 3)
#define KBC_CONTROL_KBC_EN		(1 << 0)
#define KBC_INT_FIFO_CNT_INT_STATUS	(1 << 2)
#define KBC_KPENT_VALID	(1 << 7)
#define KBC_ST_STATUS			(1 << 3)

#define KBC_RPT_DLY	20
#define KBC_RPT_RATE	4

/* keyboard controller config and state */
static struct keyb {
	struct input_config input;

	/*
	 * Information about keycode mappings. The plain_keycode array must
	 * exist but fn may be NULL in which case it is not decoded.
	 */
	const u8 *plain_keycode;	/* key code for each row / column */
	const u8 *fn_keycode;		/* ...when Fn held down */
	int fn_pos;			/* position of Fn key in key (or -1) */

	struct kbc_tegra *kbc;		/* tegra keyboard controller */
	unsigned char inited;		/* 1 if keyboard has been inited */
	unsigned int repoll_time;	/* next time to poll keyboard (us) */

	/*
	 * After init we must wait a short time before polling the keyboard.
	 * This gives the tegra keyboard controller time to react after reset
	 * and lets us grab keys pressed during reset.
	 */
	unsigned int init_dly;		/* Delay before we can read keyboard */
	unsigned long start_time;	/* Time that we inited (in us) */
} config;

/**
 * Determine if the current keypress configuration can cause key ghosting
 *
 * Matrix keyboard designs are prone to keyboard ghosting.
 * Ghosting occurs if there are 3 keys such that -
 * any 2 of the 3 keys share a row, and any 2 of them share a column.
 *
 * @param rows_val	row number for each keypress
 * @param cols_val	column number of each keypress
 * @param valid		number of valid keypresses to check
 * @return 0 if no ghosting is possible, 1 if it is
 */
static int is_ghost_key_config(int *rows_val, int *cols_val, int valid)
{
	int i, j, key_in_same_col = 0, key_in_same_row = 0;

	for (i = 0; i < valid; i++) {
		/*
		 * Find 2 keys such that one key is in the same row
		 * and the other is in the same column as the i-th key.
		 */
		for (j = i + 1; j < valid; j++) {
			if (cols_val[j] == cols_val[i])
				key_in_same_col = 1;
			if (rows_val[j] == rows_val[i])
				key_in_same_row = 1;
		}
	}

	if (key_in_same_col && key_in_same_row)
		return 1;
	else
		return 0;
}

/**
 * reads the keyboard fifo for current keypresses
 *
 * @param config	Keyboard config
 * @param fifo		Place to put fifo results
 * @return number of items put into fifo
 */
static int tegra_kbc_find_keys(struct keyb *config, int *fifo)
{
	int rows_val[KBC_MAX_KPENT], cols_val[KBC_MAX_KPENT];
	u32 kp_ent_val[(KBC_MAX_KPENT + 3) / 4];
	u32 *kp_ents = kp_ent_val;
	const u8 *keymap = config->plain_keycode;
	u32 kp_ent = 0;
	int i, valid;

	for (i = 0; i < ARRAY_SIZE(kp_ent_val); i++)
		kp_ent_val[i] = readl(&config->kbc->kp_ent[i]);

	for (i = valid = 0; i < KBC_MAX_KPENT; i++) {
		/* Get next word */
		if (!(i & 3))
			kp_ent = *kp_ents++;

		/* If this is valid, add the entry */
		if (kp_ent & KBC_KPENT_VALID) {
			int pos;

			cols_val[valid] = kp_ent & 0x7;
			rows_val[valid] = (kp_ent >> 3) & 0xf;

			/* Switch to the Fn map if available, and Fn pressed */
			pos = rows_val[valid] * KBC_MAX_COL + cols_val[valid];
			if (config->fn_keycode && pos == config->fn_pos)
				keymap = config->fn_keycode;

			/* Convert the (row, col) values into a keycode */
			fifo[valid++] = keymap[pos];
		}

		/* Shift to get next entry */
		kp_ent >>= 8;
	}

	/* For a ghost key config, ignore the keypresses for this iteration. */
	if (valid >= 3 && is_ghost_key_config(rows_val, cols_val, valid))
		return 0;

	return valid;
}

/**
 * Process all the keypress sequences in fifo and send key codes
 *
 * The fifo contains zero or more keypress sets. Each set
 * consists of from 1-8 keycodes, representing the keycodes which
 * were simultaneously pressed during that scan.
 *
 * This function works through each set and generates ASCII characters
 * for each. Not that one set may produce more than one ASCII characters -
 * for example holding down 'd' and 'f' at the same time will generate
 * two ASCII characters.
 *
 * @param config	Keyboard config
 * @param fifo_cnt	Number of entries in the keyboard fifo
 */
static void process_fifo(struct keyb *config, u32 fifo_cnt)
{
	int fifo[KBC_MAX_KPENT] = {0};
	int cnt;

	do {
		if (fifo_cnt)
			cnt = tegra_kbc_find_keys(config, fifo);
		else
			cnt = 0;
		input_send_keycodes(&config->input, fifo, cnt);
	} while (fifo_cnt--);

	udelay((fifo_cnt == 1) ? config->repoll_time : 1000);
}

/**
 * Check the keyboard controller and emit ASCII characters for any keys that
 * are pressed.
 *
 * @param config	Keyboard config
 */
static void check_for_keys(struct keyb *config)
{
	struct kbc_tegra *kbc = config->kbc;
	u32 val, ctl;

	/*
	 * Until all keys are released, defer further processing to
	 * the polling loop.
	 */
	ctl = readl(&kbc->control);
	ctl &= ~KBC_CONTROL_FIFO_CNT_INT_EN;
	writel(ctl, &kbc->control);

	/*
	 * Quickly bail out & reenable interrupts if the interrupt source
	 * wasn't fifo count threshold
	 */
	val = readl(&kbc->interrupt);
	writel(val, &kbc->interrupt);

	if (val & KBC_INT_FIFO_CNT_INT_STATUS)
		process_fifo(config, (val >> 4) & 0xf);

	ctl |= KBC_CONTROL_FIFO_CNT_INT_EN;
	writel(ctl, &kbc->control);
}

/**
 * In order to detect keys pressed on boot, wait for the hardware to
 * complete scanning the keys. This includes time to transition from
 * Wkup mode to Continous polling mode and the repoll time. We can
 * deduct the time that's already elapsed.
 *
 * @param config	Keyboard config
 */
static void kbd_wait_for_fifo_init(struct keyb *config)
{
	if (!config->inited) {
		unsigned long elapsed_time;
		long delay;

		elapsed_time = timer_get_us() - config->start_time;
		delay = config->init_dly + config->repoll_time - elapsed_time;
		if (delay > 0)
			udelay(delay);

		config->inited = 1;
	}
}

/**
 * Check the tegra keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 */
int tegra_kbc_check(struct input_config *input)
{
	kbd_wait_for_fifo_init(&config);
	check_for_keys(&config);

	return 1;
}

/**
 * Test if keys are available to be read
 *
 * @return 0 if no keys available, 1 if keys are available
 */
static int kbd_tstc(void)
{
	/* Just get input to do this for us */
	return input_tstc(&config.input);
}

/**
 * Read a key
 *
 * TODO: U-Boot wants 0 for no key, but Ctrl-@ is a valid key...
 *
 * @return ASCII key code, or 0 if no key, or -1 if error
 */
static int kbd_getc(void)
{
	/* Just get input to do this for us */
	return input_getc(&config.input);
}

/* configures keyboard GPIO registers to use the rows and columns */
static void config_kbc_gpio(struct kbc_tegra *kbc)
{
	int i;

	for (i = 0; i < KBC_MAX_GPIO; i++) {
		u32 row_cfg, col_cfg;
		u32 r_shift = 5 * (i % 6);
		u32 c_shift = 4 * (i % 8);
		u32 r_mask = 0x1f << r_shift;
		u32 c_mask = 0xf << c_shift;
		u32 r_offs = i / 6;
		u32 c_offs = i / 8;

		row_cfg = readl(&kbc->row_cfg[r_offs]);
		col_cfg = readl(&kbc->col_cfg[c_offs]);

		row_cfg &= ~r_mask;
		col_cfg &= ~c_mask;

		if (i < KBC_MAX_ROW)
			row_cfg |= ((i << 1) | 1) << r_shift;
		else
			col_cfg |= (((i - KBC_MAX_ROW) << 1) | 1) << c_shift;

		writel(row_cfg, &kbc->row_cfg[r_offs]);
		writel(col_cfg, &kbc->col_cfg[c_offs]);
	}
}

/**
 * Start up the keyboard device
 *
 * @return 0, for success
 */
static int tegra_kbc_open(void)
{
	unsigned int scan_time_rows, debounce_cnt, rpt_cnt;
	struct kbc_tegra *kbc = config.kbc;
	u32 val = 0;

	/*
	 * The time delay between two consecutive reads of the FIFO is
	 * the sum of the repeat time and the time taken for scanning
	 * the rows. There is an additional delay before the row scanning
	 * starts. The repoll delay is computed in microseconds.
	 */
	rpt_cnt = 5 * DIV_ROUND_UP(1000, KBC_CYCLE_IN_USEC);
	rpt_cnt = 32768 / 20;
	debounce_cnt = 2;
	scan_time_rows = (KBC_ROW_SCAN_TIME + debounce_cnt) * KBC_MAX_ROW;
	config.repoll_time = KBC_ROW_SCAN_DLY + scan_time_rows + rpt_cnt;
	config.repoll_time = config.repoll_time * KBC_CYCLE_IN_USEC;

	writel(rpt_cnt, &kbc->rpt_dly);

	val = KBC_DEBOUNCE_CNT_SHIFT(debounce_cnt);
	val |= KBC_FIFO_TH_CNT_SHIFT(1); /* set fifo interrupt threshold to 1 */
	val |= KBC_CONTROL_FIFO_CNT_INT_EN;  /* interrupt on FIFO threshold */
	val |= KBC_CONTROL_KBC_EN;     /* enable */

	writel(val, &kbc->control);

	config.init_dly = readl(&kbc->init_dly) * KBC_CYCLE_IN_USEC;
	config.start_time = timer_get_us();

	return 0;
}

/**
 * Create a new keycode map from some provided data
 *
 * This decodes a keycode map in the format used by the fdt, which is one
 * word per entry, with the row, col and keycode encoded in that word.
 *
 * We create a (row x col) size byte array with each entry containing the
 * keycode for that (row, col). We also search for map_keycode and return
 * its position if found (this is used for finding the Fn key).
 *
 * @param data		Keycode data
 * @param len		Number of entries in keycode table
 * @param map_keycode	Key code to find in the map
 * @param pos		Returns position of map_keycode, if found, else -1
 * @return map	Pointer to allocated map
 */
static uchar *create_keymap(u32 *data, int len, int map_keycode, int *pos)
{
	uchar *map;

	if (pos)
		*pos = -1;
	map = calloc(1, KBC_KEY_COUNT);
	if (!map) {
		debug("%s: failed to malloc %d bytes\n", __func__,
		      KBC_KEY_COUNT);
		return NULL;
	}

	for (; len >= sizeof(u32); data++, len -= 4) {
		u32 tmp = fdt32_to_cpu(*data);
		int key_code, row, col;
		int entry;

		row = (tmp >> 24) & 0xff;
		col = (tmp >> 16) & 0xff;
		key_code = tmp & 0xffff;
		entry = row * KBC_MAX_COL + col;
		map[entry] = key_code;
		if (pos && map_keycode == key_code)
			*pos = entry;
	}

	return map;
}

/**
 * Read the keyboard configuration out of the fdt.
 *
 * @param blob		FDT blob
 * @param node		Node containing compatible data
 * @param config	Keyboard config
 * @return 0 if ok, -1 on error
 */
static int fdt_decode_kbc(const void *blob, int node, struct keyb *config)
{
	const struct fdt_property *prop;
	int offset;

	config->kbc = (struct kbc_tegra *)fdtdec_get_addr(blob, node, "reg");
	if ((fdt_addr_t)config->kbc == FDT_ADDR_T_NONE)
		debug("%s: No keyboard register found\n", __func__);

	/* Check each property name for ones that we understand */
	for (offset = fdt_first_property_offset(blob, node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const char *name;
		int len;

		prop = fdt_get_property_by_offset(blob, offset, NULL);
		name = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		len = strlen(name);

		/* Name needs to match "1,<type>keymap" */
		debug("%s: property '%s'\n", __func__, name);
		if (strncmp(name, "1,", 2) || len < 8 ||
				strcmp(name + len - 6, "keymap"))
			continue;

		len -= 8;
		if (len == 0) {
			config->plain_keycode = create_keymap(
				(u32 *)prop->data, fdt32_to_cpu(prop->len),
				KEY_FN, &config->fn_pos);
		} else if (0 == strncmp(name + 2, "fn-", len)) {
			config->fn_keycode = create_keymap(
				(u32 *)prop->data, fdt32_to_cpu(prop->len),
				-1, NULL);
		} else {
			debug("%s: unrecognised property '%s'\n", __func__,
			      name);
		}

	}
	debug("%s: Decoded key maps %p, %p from fdt\n", __func__,
	      config->plain_keycode, config->fn_keycode);

	if (!config->plain_keycode) {
		debug("%s: cannot find keycode-plain map\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * Set up the tegra keyboard. This is called by the stdio device handler
 *
 * @return 0 if ok, -ve on error
 */
static int init_tegra_keyboard(void)
{
#ifdef CONFIG_OF_CONTROL
	int	node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
					  COMPAT_NVIDIA_TEGRA20_KBC);
	if (node < 0) {
		debug("%s: cannot locate keyboard node\n", __func__);
		return node;
	}
	if (fdt_decode_kbc(gd->fdt_blob, node, &config))
		return -1;
	if (config.fn_keycode) {
		if (input_add_table(&config.input, KEY_FN, -1,
					config.fn_keycode, KBC_KEY_COUNT))
			return -1;
	}
#else
#error "Tegra keyboard driver requires FDT definitions"
#endif

	/* Set up pin mux and enable the clock */
	funcmux_select(PERIPH_ID_KBC, FUNCMUX_DEFAULT);
	clock_enable(PERIPH_ID_KBC);
	config_kbc_gpio(config.kbc);

	if (tegra_kbc_open())
		return -1;
	debug("%s: Tegra keyboard ready\n", __func__);

	return 0;
}

int drv_keyboard_init(void)
{
	struct stdio_dev dev;

	if (input_init(&config.input, 0)) {
		debug("%s: Cannot set up input\n", __func__);
		return -1;
	}
	config.input.read_keys = tegra_kbc_check;

	memset(&dev, '\0', sizeof(dev));
	strcpy(dev.name, "tegra-kbc");
	dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.getc = kbd_getc;
	dev.tstc = kbd_tstc;
	dev.start = init_tegra_keyboard;

	/* Register the device. init_tegra_keyboard() will be calle soon */
	return input_stdio_register(&dev);
}
