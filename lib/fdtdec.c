/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
#include <serial.h>
#include <libfdt.h>
#include <fdtdec.h>

/* we need the generic GPIO interface here */
#include <asm-generic/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
	COMPAT(UNKNOWN, "<none>"),
	COMPAT(NVIDIA_TEGRA20_USB, "nvidia,tegra20-ehci"),
	COMPAT(SAMSUNG_EXYNOS5_SERIAL, "samsung,exynos4210-uart"),
	COMPAT(SAMSUNG_EXYNOS5_SDHCI, "samsung,exynos4210-sdhci"),
	COMPAT(COMPAT_SERIAL_TEGRA20_UART, "nvidia,tegra20-uart"),
	COMPAT(SERIAL_NS16550, "ns16550"),
	COMPAT(NVIDIA_TEGRA20_I2C, "nvidia,tegra20-i2c"),
	COMPAT(NVIDIA_TEGRA20_DVC, "nvidia,tegra20-i2c-dvc"),
	COMPAT(SMSC_LAN9215, "smsc,lan9215"),
	COMPAT(SAMSUNG_EXYNOS5_SROMC, "samsung,exynos-sromc"),
	COMPAT(SAMSUNG_EXYNOS5_MSHCI, "samsung,exynos5250-mshci"),
	COMPAT(SAMSUNG_EXYNOS_SPI, "samsung,exynos-spi"),
	COMPAT(NVIDIA_TEGRA20_KBC, "nvidia,tegra20-kbc"),
	COMPAT(SAMSUNG_EXYNOS_DMC, "samsung,exynos-dmc"),
	COMPAT(SAMSUNG_S3C2440_I2C, "samsung,s3c2440-i2c"),
	COMPAT(GOOGLE_MKBP, "google,mkbp"),
	COMPAT(GOOGLE_MKBP_KEYB, "google,mkbp-keyb"),
	COMPAT(GOOGLE_BOARD_REV, "google,board-revision"),
	COMPAT(SAMSUNG_EXYNOS_TMU, "samsung,exynos-tmu"),
	COMPAT(SAMSUNG_EXYNOS_DP, "samsung,exynos-dp"),
	COMPAT(SAMSUNG_EXYNOS_FIMD, "samsung,exynos-fb"),
	COMPAT(INFINEON_SLB9635_TPM, "infineon,slb9635-tpm"),
	COMPAT(SAMSUNG_EXYNOS_LCD, "samsung,exynos-lcd"),
	COMPAT(GOOGLE_CONST_FLAG, "google,const-flag"),
	COMPAT(GOOGLE_GPIO_FLAG, "google,gpio-flag"),
	COMPAT(GOOGLE_MKBP_FLAG, "google,mkbp-flag"),
	COMPAT(GOOGLE_SYSINFO_FLAG, "google,sysinfo-flag"),
	COMPAT(NVIDIA_TEGRA20_DISPLAY, "nvidia,tegra20-display"),
	COMPAT(INTEL_LPC, "intel,lpc"),
	COMPAT(GOOGLE_ARBITRATOR, "google,arbitrator"),
	COMPAT(SAMSUNG_EXYNOS_SOUND, "samsung,exynos-sound"),
	COMPAT(WOLFSON_WM8994_CODEC, "wolfson,wm8994-codec"),
	COMPAT(MAXIM_98095_CODEC, "maxim,max98095-codec"),
	COMPAT(GOOGLE_SATA, "google,sata"),
	COMPAT(NXP_PTN3460, "nxp,ptn3460"),
	COMPAT(SAMSUNG_EXYNOS_USB, "samsung,exynos-usb"),
	COMPAT(GOOGLE_CHROME_EC, "google,chrome-ec"),
	COMPAT(ANALOGIX_ANX7805, "analogix,anx7805"),
	COMPAT(SMSC_USB3503, "smsc,usb3503"),
};

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	return compat_names[id];
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name)
{
	const fdt_addr_t *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && (len == sizeof(fdt_addr_t) ||
			len == sizeof(fdt_addr_t) * 2)) {
		fdt_addr_t addr = fdt_addr_to_cpu(*cell);

		debug("%p\n", (void *)addr);
		return addr;
	}
	debug("(not found)\n");
	return FDT_ADDR_T_NONE;
}

s32 fdtdec_get_int(const void *blob, int node, const char *prop_name,
		s32 default_val)
{
	const s32 *cell;
	int len;

	debug("%s: %s: ", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && len >= sizeof(s32)) {
		s32 val = fdt32_to_cpu(cell[0]);

		debug("%#x (%d)\n", val, val);
		return val;
	}
	debug("(not found)\n");
	return default_val;
}

uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
		uint64_t default_val)
{
	const uint64_t *cell64;
	int length;

	cell64 = fdt_getprop(blob, node, prop_name, &length);
	if (!cell64 || length < sizeof(*cell64))
		return default_val;

	return fdt64_to_cpu(*cell64);
}

int fdtdec_get_is_enabled(const void *blob, int node)
{
	const char *cell;

	/*
	 * It should say "okay", so only allow that. Some fdts use "ok" but
	 * this is a bug. Please fix your device tree source file. See here
	 * for discussion:
	 *
	 * http://www.mail-archive.com/u-boot@lists.denx.de/msg71598.html
	 */
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return 0 == strcmp(cell, "okay");
	return 1;
}

enum fdt_compat_id fdtdec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (0 == fdt_node_check_compatible(blob, node,
				compat_names[id]))
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id)
{
	return fdt_node_offset_by_compatible(blob, node, compat_names[id]);
}

int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto)
{
#define MAX_STR_LEN 20
	char str[MAX_STR_LEN + 20];
	int node, err;

	/* snprintf() is not available */
	assert(strlen(name) < MAX_STR_LEN);
	sprintf(str, "%.*s%d", MAX_STR_LEN, name, *upto);
	node = fdt_path_offset(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	if (err)
		return -FDT_ERR_NOTFOUND;
	(*upto)++;
	return node;
}

int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	return fdtdec_add_aliases_for_id(blob, name, id, node_list, maxcount);
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	int name_len = strlen(name);
	int nodes[maxcount];
	int num_found = 0;
	int offset, node;
	int alias_node;
	int count;
	int i, j;

	/* find the alias node if present */
	alias_node = fdt_path_offset(blob, "/aliases");

	/*
	 * start with nothing, and we can assume that the root node can't
	 * match
	 */
	memset(nodes, '\0', sizeof(nodes));

	/* First find all the compatible nodes */
	node = 0;
	for (node = count = 0; node >= 0 && count < maxcount;) {
		node = fdtdec_next_compatible(blob, node, id);
		if (node >= 0)
			nodes[count++] = node;
	}
	if (node >= 0)
		debug("%s: warning: maxcount exceeded with alias '%s'\n",
		       __func__, name);

	/* Now find all the aliases */
	for (offset = fdt_first_property_offset(blob, alias_node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const struct fdt_property *prop;
		const char *path;
		int number;
		int found;

		node = 0;
		prop = fdt_get_property_by_offset(blob, offset, NULL);
		path = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		if (prop->len && 0 == strncmp(path, name, name_len))
			node = fdt_path_offset(blob, prop->data);
		if (node <= 0)
			continue;

		/* Get the alias number */
		number = simple_strtoul(path + name_len, NULL, 10);
		if (number < 0 || number >= maxcount) {
			debug("%s: warning: alias '%s' is out of range\n",
			       __func__, path);
			continue;
		}

		/* Make sure the node we found is actually in our list! */
		found = -1;
		for (j = 0; j < count; j++)
			if (nodes[j] == node) {
				found = j;
				break;
			}

		if (found == -1) {
			debug("%s: warning: alias '%s' points to a node "
				"'%s' that is missing or is not compatible "
				" with '%s'\n", __func__, path,
				fdt_get_name(blob, node, NULL),
			       compat_names[id]);
			continue;
		}

		/*
		 * Add this node to our list in the right place, and mark
		 * it as done.
		 */
		if (fdtdec_get_is_enabled(blob, node)) {
			if (node_list[number]) {
				debug("%s: warning: alias '%s' requires that "
				      "a node be placed in the list in a "
				      "position which is already filled by "
				      "node '%s'\n", __func__, path,
				      fdt_get_name(blob, node, NULL));
				continue;
			}
			node_list[number] = node;
			if (number >= num_found)
				num_found = number + 1;
		}
		nodes[found] = 0;
	}

	/* Add any nodes not mentioned by an alias */
	for (i = j = 0; i < maxcount; i++) {
		if (!node_list[i]) {
			for (; j < maxcount; j++)
				if (nodes[j] &&
					fdtdec_get_is_enabled(blob, nodes[j]))
					break;

			/* Have we run out of nodes to add? */
			if (j == maxcount)
				continue;

			node_list[i] = nodes[j++];
			if (i >= num_found)
				num_found = i + 1;
		}
	}

	return num_found;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_check_fdt(void)
{
	/* We must have an fdt */
	if (((uintptr_t)gd->fdt_blob & 3) || fdt_check_header(gd->fdt_blob))
		panic("No valid fdt found - please append one to U-Boot\n"
			"binary or define CONFIG_OF_EMBED\n");
	return 0;
}

int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name)
{
	const u32 *phandle;
	int lookup;

	phandle = fdt_getprop(blob, node, prop_name, NULL);
	if (!phandle)
		return -FDT_ERR_NOTFOUND;

	lookup = fdt_node_offset_by_phandle(blob, fdt32_to_cpu(*phandle));
	return lookup;
}

void *fdtdec_lookup_phandle_reg(const void *blob, int node,
		const char *prop_name)
{
	int lookup;

	lookup = fdtdec_lookup_phandle(blob, node, prop_name);
	if (lookup < 0)
		return NULL;
	return (void *)fdtdec_get_addr(blob, lookup, "reg");
}

/**
 * Look up a property in a node and check that it has a minimum length.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param min_len	minimum property length in bytes
 * @param err		0 if ok, or -FDT_ERR_NOTFOUND if the property is not
			found, or -FDT_ERR_BADLAYOUT if not enough data
 * @return pointer to cell, which is only valid if err == 0
 */
static const void *get_prop_check_min_len(const void *blob, int node,
		const char *prop_name, int min_len, int *err)
{
	const void *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		*err = -FDT_ERR_NOTFOUND;
	else if (len < min_len)
		*err = -FDT_ERR_BADLAYOUT;
	else
		*err = 0;
	return cell;
}

int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
		u32 *array, int count)
{
	const u32 *cell;
	int i, err = 0;

	debug("%s: %s\n", __func__, prop_name);
	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	if (!err) {
		for (i = 0; i < count; i++)
			array[i] = fdt32_to_cpu(cell[i]);
	}
	return err;
}

const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count)
{
	const u32 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	return err ? NULL : cell;
}

int fdtdec_get_bool(const void *blob, int node, const char *prop_name)
{
	const s32 *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	return cell != NULL;
}

/**
 * Decode a list of GPIOs from an FDT. This creates a list of GPIOs with no
 * terminating item.
 *
 * @param blob		FDT blob to use
 * @param node		Node to look at
 * @param prop_name	Node property name
 * @param gpio		Array of gpio elements to fill from FDT. This will be
 *			untouched if either 0 or an error is returned
 * @param max_count	Maximum number of elements allowed
 * @return number of GPIOs read if ok, -FDT_ERR_BADLAYOUT if max_count would
 * be exceeded, or -FDT_ERR_NOTFOUND if the property is missing.
 */
int fdtdec_decode_gpios(const void *blob, int node, const char *prop_name,
		struct fdt_gpio_state *gpio, int max_count)
{
	const struct fdt_property *prop;
	const u32 *cell;
	const char *name;
	int len, i;

	debug("%s: %s\n", __func__, prop_name);
	assert(max_count > 0);
	prop = fdt_get_property(blob, node, prop_name, &len);
	if (!prop) {
		debug("FDT: %s: property '%s' missing\n", __func__, prop_name);
		return -FDT_ERR_NOTFOUND;
	}

	/* We will use the name to tag the GPIO */
	name = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
	cell = (u32 *)prop->data;
	len /= sizeof(u32) * 3;		/* 3 cells per GPIO record */
	if (len > max_count) {
		debug("FDT: %s: too many GPIOs / cells for "
			"property '%s'\n", __func__, prop_name);
		return -FDT_ERR_BADLAYOUT;
	}

	/* Read out the GPIO data from the cells */
	for (i = 0; i < len; i++, cell += 3) {
		gpio[i].gpio = fdt32_to_cpu(cell[1]);
		gpio[i].flags = fdt32_to_cpu(cell[2]);
		gpio[i].name = name;
	}

	return len;
}

int fdtdec_decode_gpio(const void *blob, int node, const char *prop_name,
		struct fdt_gpio_state *gpio)
{
	int err;

	debug("%s: %s\n", __func__, prop_name);
	gpio->gpio = FDT_GPIO_NONE;
	gpio->name = NULL;
	err = fdtdec_decode_gpios(blob, node, prop_name, gpio, 1);
	return err == 1 ? 0 : err;
}

int fdtdec_get_gpio(struct fdt_gpio_state *gpio)
{
	int val;

	if (!fdt_gpio_isvalid(gpio))
		return -1;

	val = gpio_get_value(gpio->gpio);
	return gpio->flags & FDT_GPIO_ACTIVE_LOW ? val ^ 1 : val;
}

int fdtdec_set_gpio(struct fdt_gpio_state *gpio, int val)
{
	if (!fdt_gpio_isvalid(gpio))
		return -1;

	val = gpio->flags & FDT_GPIO_ACTIVE_LOW ? val ^ 1 : val;
	return gpio_set_value(gpio->gpio, val);
}

int fdtdec_setup_gpio(struct fdt_gpio_state *gpio)
{
	/*
	 * Return success if there is no GPIO defined. This is used for
	 * optional GPIOs)
	 */
	if (!fdt_gpio_isvalid(gpio))
		return 0;

	if (gpio_request(gpio->gpio, gpio->name))
		return -1;
	return 0;
}

int fdtdec_decode_periph_id(const void *blob, int node)
{
	u32 cell[2];
	int err;

	err = fdtdec_get_int_array(blob, node, "clocks", cell,
				   ARRAY_SIZE(cell));
	if (err)
		return -1;

	return cell[1];
}

int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
		u8 *array, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (!err)
		memcpy(array, cell, count);
	return err;
}

const u8 *fdtdec_locate_byte_array(const void *blob, int node,
			     const char *prop_name, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (err)
		return NULL;
	return cell;
}

int fdtdec_get_config_int(const void *blob, const char *prop_name,
		int default_val)
{
	int config_node;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return default_val;
	return fdtdec_get_int(blob, config_node, prop_name, default_val);
}

int fdtdec_get_config_bool(const void *blob, const char *prop_name)
{
	int config_node;
	const void *prop;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return 0;
	prop = fdt_get_property(blob, config_node, prop_name, NULL);

	return prop != NULL;
}

char *fdtdec_get_config_string(const void *blob, const char *prop_name)
{
        const char *nodep;
        int nodeoffset;
        int len;

        debug("%s: %s\n", __func__, prop_name);
        nodeoffset = fdt_path_offset(blob, "/config");
        if (nodeoffset < 0)
                return NULL;

        nodep = fdt_getprop(blob, nodeoffset, prop_name, &len);
        if (!nodep)
                return NULL;

        return (char *)nodep;
}

int fdtdec_decode_region(const void *blob, int node,
		const char *prop_name, void **ptrp, size_t *size)
{
	const fdt_addr_t *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell || (len != sizeof(fdt_addr_t) * 2))
		return -1;

	*ptrp = (void *)fdt_addr_to_cpu(*cell);
	*size = fdt_size_to_cpu(cell[1]);
	debug("%s: size=%zx\n", __func__, *size);
	return 0;
}
