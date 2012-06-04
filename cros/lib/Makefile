#
# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Alternatively, this software may be distributed under the terms of the
# GNU General Public License ("GPL") version 2 as published by the Free
# Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libcros.o

COBJS-$(CONFIG_CHROMEOS) += boot_kernel.o
# TODO(clchiou): Work out the conflicts with coreboot/cros_gpio.o
ifndef CONFIG_SYS_COREBOOT
COBJS-$(CONFIG_CHROMEOS) += cros_gpio_fdt.o
endif
COBJS-$(CONFIG_CHROMEOS) += cros_gpio_debug.o
COBJS-$(CONFIG_CHROMEOS) += cros_init.o
COBJS-$(CONFIG_CHROMEOS) += crossystem_data.o
ifeq ($(CONFIG_CHROMEOS),y)
COBJS-$(CONFIG_OF_CONTROL) += cros_fdtdec.o
endif
COBJS-$(CONFIG_CHROMEOS) += firmware_storage_spi.o
COBJS-$(CONFIG_CHROMEOS) += fmap.o
COBJS-$(CONFIG_CHROMEOS) += gbb.o
COBJS-$(CONFIG_CHROMEOS) += memory_wipe.o

# TODO(sjg): This MMC code is not needed as yet, and needs slight changes
# to build now
#COBJS-$(CONFIG_CHROMEOS) += firmware_storage_twostop.o

COBJS	:= $(COBJS-y)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
