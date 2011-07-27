/* Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tpm.h>

int tis_init(void)
{
	printf("tis_init called but not implemented.\n");
	return 1;
}

int tis_open(void)
{
	printf("tis_open called but not implemented.\n");
	return 1;
}

int tis_close(void)
{
	printf("tis_close called but not implemented.\n");
	return 1;
}

int tis_sendrecv(const uint8_t *sendbuf, size_t send_size, uint8_t *recvbuf,
                        size_t *recv_len)
{
	printf("tis_sendrecv called but not implemented.\n");
	return 1;
}
