/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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

/* board early intialization */
.globl early_board_init
early_board_init:
	/*
	 * This could be the second u-boot phase starting, and when under
	 * coreboot control there have been no explicit zeroing of the bss
	 * segment. Let's do it here.
	 */

	/* start of area to clean */
	movl	$__bss_start, %edi

	/* calculate number of words to set to zero */
	movl	$__bss_end, %ecx
	subl	%edi, %ecx
	shr	$2, %ecx

	/* do it */
	cld
	xorl	%eax, %eax
	rep 	stosl
	jmp	early_board_init_ret
