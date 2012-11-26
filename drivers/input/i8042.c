/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* i8042.c - Intel 8042 keyboard driver routines */

/* includes */

#include <common.h>

#ifdef CONFIG_USE_CPCIDVI
extern u8 gt_cpcidvi_in8(u32 offset);
extern void gt_cpcidvi_out8(u32 offset, u8 data);

#define in8(a)	   gt_cpcidvi_in8(a)
#define out8(a, b) gt_cpcidvi_out8(a, b)
#endif

#include <i8042.h>

/* defines */

#ifdef CONFIG_CONSOLE_CURSOR
extern void console_cursor(int state);
static int blinkCount = CONFIG_SYS_CONSOLE_BLINK_COUNT;
static int cursor_state;
#endif

/*
 * Use a simple FIFO to convert some keys into escape sequences and to handle
 * tstc vs getc.  The FIFO length must be a power of two.  Minimal function
 * requires that it be large enough to contain the generated escape sequence for
 * one key.
 */
#define KBC_FIFO_LENGTH	(1 << 3)

static int kbc_fifo[KBC_FIFO_LENGTH];
static int kbc_fifo_read;
static int kbc_fifo_write;


/* locals */

static int  kbd_mapping	 = KBD_US;	/* default US keyboard */
static int  kbd_flags	 = NORMAL;	/* after reset */
static int  kbd_state;			/* unshift code */
static int  kbd_key_release;		/* key release in progress */

static int  kbd_conv_usb(unsigned char scan_code);
static void kbd_led_set(int ps2_leds);
static int  i8042_ready(void);
static int  kbd_reset(void);


/*
 * Translate PS/2 Keyboard Scan Code Set 1 values into the USB Scan Codes
 *
 * Reasons to use USB scan codes:
 *	o	Standard
 *	o	Simple conversion to ASCII / ANSI 3.64
 *	o	Ability to share complex processing / state code
 *	o	Shared international keymaps in higher level code
 *
 * Extended codes (pseudo PS/2 scan codes >= 0x80) are positionally
 * translated from "0xE0 + scan code" using a linear table to save
 * data size; we intentionally do not support odd multibyte sequences.
 */

/* Scan codes following a 0xE0 scan code... */
static unsigned char ext_0_key_usb[] = {
	0x00,	/* UNSUPPORTED: Print Screen (multibyte) */
	0x00,	/* UNSUPPORTED: Pause (multibyte) */
	0x00,	/* RESERVED: Keypad 5 (undefined action key) */

	0x5E,	/* Power */
	0x5F,	/* RESERVED: Sleep (maps to Keyboard Power) */
	0x63,	/* RESERVED: Wake (maps to Keyboard Power) */

	0x35,	/* Keypad Enter */
	0x1C,	/* Keypad / */
	0x5B,	/* Left GUI */
	0x1D,	/* Right Control */
	0x38,	/* Right Alt / Alt GR */
	0x5C,	/* Right GUI */

	0x52,	/* Insert */
	0x47,	/* Home */
	0x49,	/* Page Up */
	0x53,	/* Delete Forward */
	0x4F,	/* End */
	0x51,	/* Page Down */
	0x4D,	/* Right */
	0x4B,	/* Left */
	0x50,	/* Down */
	0x48	/* Up */
};
#define EXT_0_KEY_USB_SIZE	ARRAY_SIZE(ext_0_key_usb)

static unsigned char sc_to_usb[0x80 + EXT_0_KEY_USB_SIZE] = {
	0x00, 0x29, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, /* scan 00-07 */
	0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x2A, 0x2B, /* scan 08-0F */
	0x14, 0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, /* scan 10-17 */
	0x12, 0x13, 0x2F, 0x30, 0x28, 0xE0, 0x04, 0x16, /* scan 18-1F */
	0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, /* scan 20-27 */
	0x34, 0x35, 0xE1, 0x31, 0x1D, 0x1B, 0x06, 0x19, /* scan 28-2F */
	0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0xE5, 0x55, /* scan 30-37 */
	0xE2, 0x2C, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, /* scan 38-3F */
	0x3F, 0x40, 0x41, 0x42, 0x43, 0x53, 0x47, 0x5F, /* scan 40-47 */
	0x60, 0x61, 0x56, 0x5C, 0x5D, 0x5E, 0x57, 0x59, /* scan 48-4F */
	0x5A, 0x5B, 0x62, 0x63, 0x00, 0x00, 0x00, 0x44, /* scan 50-57 */
	0x45, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, /* scan 58-5F */
	0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scan 60-67 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scan 68-6F */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scan 70-77 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* scan 78-7F */
	0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x58, 0x54, /* extended*/
	0xE3, 0xE4, 0xE6, 0x5C, 0x49, 0x4A, 0x4B, 0x4C, /* extended*/
	0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52              /* extended*/
};

/*
 * USB scan code to ANSI 3.64 escape sequence table.  This table is
 * incomplete in that it does not include all possible extra keys, nor
 * the 0xE1 report keys.
 */
static struct map364 {
	unsigned char usb_scan_code;
	char *string;
} usb_to_ansi364[] = {
	{ 0x49, "\033[2~"},	/* Insert */
	{ 0x4A, "\033[0H"},	/* Home */
	{ 0x4B, "\033[5~"},	/* Page Up */
	{ 0x4C, "\033[3~"},	/* Delete Forward */
	{ 0x4D, "\033[0F"},	/* End */
	{ 0x4E, "\033[3~"},	/* Page Down */
	{ 0x4F, "\033[C"},	/* Right */
	{ 0x50, "\033[D"},	/* Left */
	{ 0x51, "\033[B"},	/* Down */
	{ 0x52, "\033[A"},	/* Up */
	{ 0x3A, "\033[OP" },	/* F1 */
	{ 0x3B, "\033[OQ" },	/* F2 */
	{ 0x3C, "\033[OR" },	/* F3 */
	{ 0x3D, "\033[OS" },	/* F4 */
	{ 0x3E, "\033[15~" },	/* F5 */
	{ 0x3F, "\033[17~" },	/* F6 */
	{ 0x40, "\033[18~" },	/* F7 */
	{ 0x41, "\033[19~" },	/* F8 */
	{ 0x42, "\033[20~" },	/* F9 */
	{ 0x43, "\033[21~" },	/* F10 */
	{ 0x44, "\033[23~" },	/* F11 */
	{ 0x45, "\033[24~" }	/* F12 */
};
#define USB_TO_ANSI364_SIZE	ARRAY_SIZE(usb_to_ansi364)

/* Modifier bits */
#define LEFT_CNTR	(1 << 0)
#define LEFT_SHIFT	(1 << 1)
#define LEFT_ALT	(1 << 2)
#define LEFT_GUI	(1 << 3)
#define RIGHT_CNTR	(1 << 4)
#define RIGHT_SHIFT	(1 << 5)
#define RIGHT_ALT	(1 << 6)
#define RIGHT_GUI	(1 << 7)

/* USB locking modifier keys */
#define USB_KEY_NUM_LOCK	0x53
#define USB_KEY_CAPS_LOCK	0x39
#define USB_KEY_SCROLL_LOCK	0x47

/* Masking for lower to upper case conversion */
#define CAPITAL_MASK	0x20

/* USB LED bit order; the mask is to avoid sending unknown bits */
#define USB_KBD_NUMLOCK		(1 << 0)
#define USB_KBD_CAPSLOCK	(1 << 1)
#define USB_KBD_SCROLLLOCK	(1 << 2)
#define USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

/*
 * Convert from USB "Set LEDs" command bit order to PS/2 Port 0x60
 * Command 0xED bit order.
 */
static unsigned char usb_kbd_led_to_ps2(unsigned char usb_led_bits)
{
	return (((usb_led_bits) << 1) & 0x03) |
	       ((usb_led_bits & USB_KBD_SCROLLLOCK) ? 0x01 : 0x00);
}


static unsigned char bits_modifiers;	/* individual modifier keys */
static unsigned char bits_state;	/* modifier state (includes LED bits) */

/* States for the FSA that manages input of PS/2 scan codes */
enum ps2_kbd_state_t {
	KS_BASE = 0,	/* Base state */
	KS_EXTENDED_0,	/* Extended (E0) scan code byte expected */
	KS_EXTENDED_1A,	/* Extended (E1) scan code first byte expected */
	KS_EXTENDED_1B,	/* Extended (E1) scan code second byte expected */
};

static enum ps2_kbd_state_t ps2_kbd_state = KS_BASE;

/* Keyboard maps */
static const unsigned char usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
static const unsigned char usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};
static const unsigned char usb_kbd_numkey_ctrled[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\n', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};



/*
 * Simple FIFO for conversion of input characters to escape sequences
 */

/* Return true if there are no characters in the FIFO. */
static int kbd_fifo_empty(void)
{
	return kbc_fifo_read == kbc_fifo_write;
}

/* Return number of characters of free space in the FIFO. */
static int kbd_fifo_free_space(void)
{
	return KBC_FIFO_LENGTH - (kbc_fifo_write - kbc_fifo_read);
}

/*
 * Insert a character into the FIFO.  Calling this function when the FIFO is
 * full will overwrite the oldest character in the FIFO.
 */
static void kbd_fifo_insert(int key)
{
	int index = kbc_fifo_write & (KBC_FIFO_LENGTH - 1);

	/* Special case for unregocnized keys */
	if (key == 0x00)
		return;

	assert(kbd_fifo_free_space() > 0);

	kbc_fifo[index] = key;

	kbc_fifo_write++;
}

/*
 * Remove a character from the FIFO, it is an error to call this function when
 * the FIFO is empty.
 */
static int kbd_fifo_remove(void)
{
	int index = kbc_fifo_read & (KBC_FIFO_LENGTH - 1);
	int key   = kbc_fifo[index];

	assert(!kbd_fifo_empty());

	kbc_fifo_read++;

	return key;
}

/******************************************************************************/

static int kbd_controller_present(void)
{
	return in8(I8042_STATUS_REG) != 0xff;
}

/*
 * Implement a weak default function for boards that optionally
 * need to skip the i8042 initialization.
 */
int __board_i8042_skip(void)
{
	/* As default, don't skip */
	return 0;
}
int board_i8042_skip(void) __attribute__((weak, alias("__board_i8042_skip")));

/*******************************************************************************
 *
 * i8042_flush - flush all buffer from EC to host.
 *               The delay is to give EC some time to fill next byte.
 */
void i8042_flush(void)
{
	int timeout;

	while (1) {
		timeout = 100;  /* wait for no longer than 100us */
		while (timeout > 0 && (in8(I8042_STATUS_REG) & 0x01) == 0) {
			udelay(1);
			timeout--;
		}

		/* Try to pull next byte if not timeout. */
		if (in8(I8042_STATUS_REG) & 0x01)
			in8(I8042_DATA_REG);
		else
			return;
	}
}

/*******************************************************************************
 *
 * i8042_disable - Disables the keyboard so that key stroke no longer generates
 *                 scancode to host.
 */
int i8042_disable(void)
{
	if (i8042_ready() == 0)
		return -1;

	/* Disable keyboard */
	out8(I8042_COMMAND_REG, 0xad);

	if (i8042_ready() == 0)
		return -1;

	return 0;
}

/*******************************************************************************
 *
 * i8042_kbd_init - reset keyboard and init state flags
 */
int i8042_kbd_init(void)
{
	int keymap, try;
	char *penv;

	if (!kbd_controller_present() || board_i8042_skip())
		return -1;

#ifdef CONFIG_USE_CPCIDVI
	penv = getenv("console");
	if (penv != NULL) {
		if (strncmp(penv, "serial", 7) == 0)
			return -1;
	}
#endif
	/* Init keyboard device (default US layout) */
	keymap = KBD_US;
	penv = getenv("keymap");
	if (penv != NULL) {
		if (strncmp(penv, "de", 3) == 0)
			keymap = KBD_GER;
	}

	for (try = 0; try < KBD_RESET_TRIES; try++) {
		if (kbd_reset() == 0) {
			kbd_mapping = keymap;
			kbd_flags   = NORMAL;
			kbd_state   = 0;
			kbd_led_set(0);	  /* Start with LEDs off */
			return 0;
		}
	}
	return -1;
}


/*******************************************************************************
 *
 * i8042_tstc - test if keyboard input is available
 *		option: cursor blinking if called in a loop
 */
int i8042_tstc(void)
{
	unsigned char scan_code = 0;
	int kbd_input;

	/* If there's something in the fifo, we're done. */
	if (!kbd_fifo_empty())
		return 1;

#ifdef CONFIG_CONSOLE_CURSOR
	if (--blinkCount == 0) {
		cursor_state ^= 1;
		console_cursor(cursor_state);
		blinkCount = CONFIG_SYS_CONSOLE_BLINK_COUNT;
		udelay(10);
	}
#endif

	if ((in8(I8042_STATUS_REG) & I8042_STR_OBF) == 0) {
		return 0;
	} else {
		scan_code = in8(I8042_DATA_REG);
		if (scan_code == 0xfa)
			return 0;

		kbd_input = kbd_conv_usb(scan_code);
		if (kbd_input != -1) {
			kbd_fifo_insert(kbd_input);
			return 1;
		}
	}
	return 0;
}


/******************************************************************************/

/* option: turn on/off cursor while waiting */
static void kbd_fetch_char(int test)
{
	unsigned char scan_code;
	int kbd_input = -1;

	while (kbd_input == -1) {
		while ((in8(I8042_STATUS_REG) & I8042_STR_OBF) == 0) {
#ifdef CONFIG_CONSOLE_CURSOR
			if (--blinkCount == 0) {
				cursor_state ^= 1;
				console_cursor(cursor_state);
				blinkCount = CONFIG_SYS_CONSOLE_BLINK_COUNT;
			}
			udelay(10);
#endif
		}

		scan_code = in8(I8042_DATA_REG);

		if (scan_code != 0xfa) {
			kbd_input = kbd_conv_usb(scan_code);
			if (kbd_input != -1)
				break;
		}
	}

	kbd_fifo_insert(kbd_input);
}

/*******************************************************************************
 *
 * i8042_getc - wait till keyboard input is available
 */
int i8042_getc(void)
{
	if (kbd_fifo_empty())
		kbd_fetch_char(0);

	return kbd_fifo_remove();
}


/******************************************************************************/

static unsigned char scan_code_convert_ps2_usb(unsigned char scan_code)
{
	unsigned char cooked_scan_code = (scan_code & 0x7F);
	unsigned char usb_scan_code = 0x00;	/* keep compiler happy */
	int offset;

	/* Reset the FSA on illegal scan codes */
	if (ps2_kbd_state != KS_BASE &&
	    (scan_code == 0xE0 || scan_code == 0xE1)) {
		ps2_kbd_state = KS_BASE;
		return 0x00;
	}

	/*
	 * If the high bit is set, this is a break code, otherwise it is a
	 * make code.
	 */
	if (scan_code & 0x80)
		kbd_key_release = 1;
	else
		kbd_key_release = 0;

	/* PS/2 Scan code FSA */
	switch (ps2_kbd_state) {
	case KS_BASE:
		if (scan_code == 0xE0) {
			ps2_kbd_state = KS_EXTENDED_0;
			usb_scan_code = 0x00;
			break;
		}
		/* 0xE1 is Only used by the Pause / Break key */
		if (scan_code == 0xE1) {
			ps2_kbd_state = KS_EXTENDED_1A;
			usb_scan_code = 0x00;
			break;
		}

		/*
		 * Convert the cooked scan code.  For unrecognized scan
		 * codes, we have explicit 0x00 values in the table.
		 */
		usb_scan_code = sc_to_usb[cooked_scan_code];
		break;

	case KS_EXTENDED_0:

		for (offset = 0; offset < EXT_0_KEY_USB_SIZE; offset++) {
			if (cooked_scan_code != ext_0_key_usb[offset])
				continue;
			usb_scan_code = sc_to_usb[0x80 + offset];
			break;
		}

		/*
		 * Note: Reset to base here precludes parsing Print Screen
		 *       sequence 0xE0 0x2A 0xE0 0x37, and inverse sequence
		 *       0xE0 0xB7 0xE0 0xAA (Note inverse break scan code
		 *       order if you intend to handle this case!).
		 */
		ps2_kbd_state = KS_BASE;

		/* Unrecognized scan code following 0xE0 */
		if (offset == EXT_0_KEY_USB_SIZE)
			usb_scan_code = 0x00;
		break;

	case KS_EXTENDED_1A:
	case KS_EXTENDED_1B:
		/* The Pause key sends 2 bytes following the state change */
		if (ps2_kbd_state == KS_EXTENDED_1A)
			ps2_kbd_state = KS_EXTENDED_1B;
		else
			ps2_kbd_state = KS_BASE;

		/*
		 * Note: The Pause key make sequence is 0xE1 0x1D 0x45; it
		 *       is always immediately followed by its break sequence
		 *       of 0xE1, 0x9D, 0xC5; there is no persistent make
		 *       state duration for the Pause key, for which reason
		 *       it is normally treated as a toggle by upper level
		 *       software.  Handling is not recommended.
		 */
		usb_scan_code = 0x00;
		break;
	}

	/* Locking modifier keys notify on press and again on release. */
	if (!kbd_key_release) {
		unsigned char old_bits_state = bits_state;

		switch (usb_scan_code) {
		case USB_KEY_NUM_LOCK:
			bits_state ^= USB_KBD_NUMLOCK;
			break;
		case USB_KEY_CAPS_LOCK:
			bits_state ^= USB_KBD_CAPSLOCK;
			break;
		case USB_KEY_SCROLL_LOCK:
			bits_state ^= USB_KBD_SCROLLLOCK;
			break;
		}
		/* If we changed any bits, poke the LEDs. */
		if (old_bits_state != bits_state)
			kbd_led_set(usb_kbd_led_to_ps2(bits_state));
	}

	return usb_scan_code;
}

/*
 * For a given USB scan code, cook the value into zero or more character
 * codes.  Because Ctrl-Space is NUL, we return an integer value which may
 * be -1 in the case that the key doesn't result in a character code.
 */
static int usb_cook_scan_code(unsigned char usb_scan_code)
{
	int index;

	/*
	 * Handle in-band modifier keys.
	 *
	 * Note: These are not generated by USB keyboards with working
	 *       USB 1.1 HID compliant firmware, but broken firmware
	 *	 exists.  This conversion is therefore safe for all keyboards.
	 */
	if (usb_scan_code >= 0xE0 && usb_scan_code <= 0xE7) {
		unsigned char bit = (1 << (usb_scan_code - 0xE0));

		if (kbd_key_release)
			bits_modifiers &= ~bit;
		else
			bits_modifiers |= bit;

		return -1;
	}

	/*
	 * Key up only changes modifier state.
	 *
	 * LATER: Add shadow matrix support so u-boot clients can query the
	 *	  state of keys.  This will allow the upper level code to
	 *	  implement things like continuing to hold the power button
	 *	  to enter DFU mode, or use Ctrl-Alt-T to enter target disk
	 *	  node, etc..
	 */
	if (kbd_key_release)
		return -1;

	/* Handle special sequence keys; this doesn't need performance */
	for (index = 0; index < USB_TO_ANSI364_SIZE; index++) {
		char *p;
		if (usb_scan_code != usb_to_ansi364[index].usb_scan_code)
			continue;
		/* Stuff the FIFO ourselves; sign coerce static strings */
		for (p = usb_to_ansi364[index].string; *p; p++)
			kbd_fifo_insert((unsigned char)*p);
		/* If not, this is an unsupported key */
		return -1;
	}

	/* Handle numeric keypad keys */
	if ((usb_scan_code > 0x1d) && (usb_scan_code < 0x39)) {
		int shifted;

		if (bits_modifiers & (LEFT_CNTR | RIGHT_CNTR))
			return usb_kbd_numkey_ctrled[usb_scan_code - 0x1e];

		/* Shift inverts Num Lock state */
		shifted = (bits_modifiers & (LEFT_SHIFT | RIGHT_SHIFT)) != 0;
		if (bits_state & USB_KBD_NUMLOCK)
			shifted = !shifted;

		if (shifted)
			return usb_kbd_numkey_shifted[usb_scan_code - 0x1e];
		else
			return usb_kbd_numkey[usb_scan_code - 0x1e];
	}

	/* Handle control keys */
	/*
	 * Note: This is a compromise; it gets the right values for the
	 *       right keys, but for unexpected keys, the control character
	 *       sent will depend on the usb_scan_code.  This approximates
	 *       PS/2 historical behaviour.
	 */
	if (bits_modifiers & (LEFT_CNTR | RIGHT_CNTR))
		return (usb_scan_code - 0x03) & 0x1F;

	/* Handle ordinary alphanumerics */
	if ((usb_scan_code > 0x03) && (usb_scan_code <= 0x1D)) {
		int keycode = usb_scan_code - 0x04 + 'a';

		/* Caps Lock */
		if (bits_state & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		/* Shift inverts Caps Lock state */
		if (bits_modifiers & (LEFT_SHIFT | RIGHT_SHIFT)) {
			if (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			else
				keycode |= CAPITAL_MASK;
		}
		return keycode;
	}
	return -1;
}

static int kbd_conv_usb(unsigned char scan_code)
{
	unsigned char usbcode;

	usbcode = scan_code_convert_ps2_usb(scan_code);
	return usb_cook_scan_code(usbcode);
}


/******************************************************************************/

static void kbd_led_set(int ps2_leds)
{
	i8042_ready();
	out8(I8042_DATA_REG, I8042_DATA_LED_WRITE);	/* SET LED command */
	i8042_ready();
	out8(I8042_DATA_REG, (ps2_leds & I8042_LED_MASK)); /* LED bits only */
}


/*******************************************************************************
 *
 * i8042_ready - wait for the i8042 to finish processing commands/data
 *		 or a timeout
 */
static int i8042_ready(void)
{
	int kbdTimeout = KBD_TIMEOUT * 1000;

	while ((in8(I8042_STATUS_REG) & I8042_STR_IBF) && kbdTimeout--)
		udelay(1);

	return kbdTimeout != -1;
}

/******************************************************************************/

static int kbd_reset(void)
{
	if (i8042_ready() == 0)
		return -1;

	out8(I8042_DATA_REG, I8042_DATA_KBD_RESET);

	if (i8042_ready() == 0)
		return -1;

	out8(I8042_COMMAND_REG, I8042_CMD_SET_CMD_BYTE);

	if (i8042_ready() == 0)
		return -1;

	/*
	 * Command byte - bitmask:
	 *
	 * 7   0 = unused; set to 0
	 * 6   0 = no scan code conversion
	 *     1 = standard scan code conversion
	 * 5   0 = check parity, with scan code conversion
	 *     1 = ignore parity, no scan code coversion
	 * 4   0 = Enable keyboard
	 *     1 = Disable keyboard by forcing CLK low
	 * 3   0 = Normal keyboard inhibit function
	 *     1 = Override keyboard inhibit function (used for POST)
	 * 2   0 = System flag status bit indicates reset by power on
	 *     1 = System flag after successful controller self test
	 * 1   0 = unused; set to 0
	 * 0   0 = Do not interrupt on output buffer full
	 *     1 = Output buffer full causes interrupt on IRQ 1
	 */
	out8(I8042_DATA_REG, 0x45);

	if (i8042_ready() == 0)
		return -1;

	out8(I8042_COMMAND_REG, I8042_CMD_ENABLE_KBD);

	if (i8042_ready() == 0)
		return -1;

	return 0;
}
