/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <asm/errno.h>
#include <common.h>
#include <fdtdec.h>
#include <mkbp.h>
#include <tps65090.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	MAX_FET_NUM	= 7,
	MAX_CTRL_READ_TRIES = 5,
};

/* How long we wait for a TPS65090 FET to turn on */
#define FET_ENABLE_TIMEOUT 100 /* ms */

static struct mkbp_dev *mkbp_dev;

/**
 * Checks for a valid FET number
 *
 * @param fet_id	FET number to check
 * @return 0 if ok, -1 if FET value is out of range
 */
static int mkbp_tps_check_fet(unsigned int fet_id)
{
	if (fet_id == 0 || fet_id > MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
				fet_id, MAX_FET_NUM);
		return -1;
	}

	return 0;
}

/**
 * Set the power state for a FET
 *
 * @param fet_id	Fet number to set (1..MAX_FET_NUM)
 * @param set		1 to power on FET, 0 to power off
 * @return FET_ERR_COMMS if we got a comms error, FET_ERR_NOT_READY if the
 * FET failed to change state. If all is ok, returns 0.
 */
static int mkbp_tps_fet_set(unsigned int fet_id, int set)
{
	int retry;
	uchar reg;

	if (!mkbp_dev) {
		debug("%s: no mkbp device\n", __func__);
		return -1;
	}

	if (mkbp_set_ldo(mkbp_dev, fet_id, set)) {
		debug("%s: cannot set TPS65090 FET through EC\n",
			__func__);
		return FET_ERR_COMMS;
	}

	/* Try reading until we get a result */
	for (retry = 0; retry < MAX_CTRL_READ_TRIES; retry++) {
		if (mkbp_get_ldo(mkbp_dev, fet_id, &reg)) {
			debug("%s: cannot read FET through EC\n",
				__func__);
			return FET_ERR_COMMS;
		}
		/* Check that the fet went into the expected state */
		if (!!(reg & EC_LDO_STATE_ON) == set)
			return 0;

		mdelay(1);
	}
	debug("FET %d: Power good should have set to %d but is %d\n",
	      fet_id, set, reg);
	return FET_ERR_NOT_READY;
}

int mkbp_tps65090_fet_enable(unsigned int fet_id)
{
	int loops;
	ulong start;
	int ret = 0;

	if (mkbp_tps_check_fet(fet_id))
		return -1;
	start = get_timer(0);
	for (loops = 0; ; loops++) {
		ret = mkbp_tps_fet_set(fet_id, 1);
		if (!ret)
			break;

		if (get_timer(start) > FET_ENABLE_TIMEOUT)
			break;

		/* Turn it off and try again until we time out */
		mkbp_tps_fet_set(fet_id, 0);
	}

	if (ret) {
		debug("%s: FET%d failed to power on: time=%lums, loops=%d\n",
	      __func__, fet_id, get_timer(start), loops);
	} else if (loops) {
		debug("%s: FET%d powered on after %lums, loops=%d\n",
		      __func__, fet_id, get_timer(start), loops);
	}
	/*
	 * Unfortunately, there are some conditions where the power
	 * good bit will be 0, but the fet still comes up. One such
	 * case occurs with the lcd backlight. We'll just return 0 here
	 * and assume that the fet will eventually come up.
	 */
	if (ret == FET_ERR_NOT_READY)
		ret = 0;

	return ret;
}

int mkbp_tps65090_fet_disable(unsigned int fet_id)
{
	if (mkbp_tps_check_fet(fet_id))
		return -1;
	return mkbp_tps_fet_set(fet_id, 0);
}

int mkbp_tps65090_fet_is_enabled(unsigned int fet_id)
{
	unsigned char reg;

	if (mkbp_tps_check_fet(fet_id))
		return -1;

	if (mkbp_get_ldo(mkbp_dev, fet_id, &reg)) {
		debug("%s: cannot read FET through EC\n", __func__);
		return -2;
	}
	return reg & EC_LDO_STATE_ON;
}

int mkbp_tps65090_init(void)
{
	const void *blob = gd->fdt_blob;
	int node, parent;

	node = fdtdec_next_compatible(blob, 0, COMPAT_TI_TPS65090);
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -ENOENT;
	}
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	if (fdt_node_check_compatible(blob, parent,
			fdtdec_get_compatible(COMPAT_GOOGLE_MKBP))) {
		debug("%s: TPS65090 not behind MKBP\n", __func__);
		return -ENOENT;
	}

	mkbp_dev = board_get_mkbp_dev();
	if (!mkbp_dev) {
		debug("%s: no mkbp device: cannot init tps65090\n",
			__func__);
		return -1;
	}

	debug("%s: accessing tps65090 through MKBP\n", __func__);
	return 0;
}
