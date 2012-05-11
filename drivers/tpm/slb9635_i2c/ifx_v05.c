/* Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <config.h>
#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include "tddl.h"

DECLARE_GLOBAL_DATA_PTR;

/* TPM configuration */
struct tpm_dev {
	int i2c_bus;
	int slave_addr;
} g_tpm_dev;

/**
 * Decode TPM configuration.
 *
 * @param dev	Returns a configuration of TPM device
 * @return 0 if ok, -1 on error
 */
static int tpm_decode_config(struct tpm_dev *dev)
{
#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;
	int node, parent;
	int i2c_bus;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9635_TPM);
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -1;
	}
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}
	i2c_bus = i2c_get_bus_num_fdt(blob, parent);
	if (i2c_bus < 0)
		return -1;
	dev->i2c_bus = i2c_bus;
	dev->slave_addr = fdtdec_get_addr(blob, node, "reg");
#else
	dev->i2c_bus = CONFIG_INFINEON_TPM_I2C_BUS;
	dev->slave_addr = TPM_V05_ADDR;
#endif
	return 0;
}

int tpm_init_v05(void)
{
	int rc;

	rc = tpm_decode_config(&g_tpm_dev);
	if (rc)
		return -1;

	rc = i2c_set_bus_num(g_tpm_dev.i2c_bus);
	if (rc) {
		debug("%s: fail: i2c_set_bus_num(0x%x) return %d\n", __func__,
				g_tpm_dev.i2c_bus, rc);
		return rc;
	}
	/* request for waking up device */
	rc = i2c_probe(g_tpm_dev.slave_addr);
	if (rc == 0)
		return 0;

	/* do the probing job */
	rc = i2c_probe(g_tpm_dev.slave_addr);
	if (rc == 0)
		return 0;

	debug("%s: fail: i2c_probe(0x%x) return %d\n", __func__,
			g_tpm_dev.slave_addr, rc);
	return rc;
}

int tpm_open_v05(void)
{
	if (TDDL_Open(g_tpm_dev.slave_addr))
		return -1;
	return 0;
}

int tpm_close_v05(void)
{
	if (TDDL_Close())
		return -1;
	return 0;
}

int tpm_sendrecv_v05(const uint8_t *sendbuf, size_t buf_size,
	uint8_t *recvbuf, size_t *recv_len)
{
	if (TDDL_TransmitData((uint8_t*)sendbuf, (uint32_t)buf_size, recvbuf,
		(uint32_t*)recv_len))
		return -1;
	return 0;
}

