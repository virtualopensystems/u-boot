#!/bin/sh -e

export CROSS_COMPILE=arm-linux-gnueabihf-

TEXT_START=$(grep CONFIG_SYS_TEXT_BASE include/configs/exynos5-common.h | awk '{ print $3 }')
NV_NAME=nv_uboot-snow
UIMAGE=${NV_NAME}.uimage
KPART=${NV_NAME}.kpart

# cleanup previous build binaries 
rm -f ${NV_NAME}*

# clean, configure and build
make distclean
make snow_config
make -j4 all

# Make it look like an image U-Boot will like:
# The "-a" and "-e" here are the "CONFIG_SYS_TEXT_BASE" from
# include/configs/exynos5-common.h
tools/mkimage \
-A arm \
-O linux \
-T kernel \
-C none \
-a "${TEXT_START}" -e "${TEXT_START}" \
-n "Non-verified u-boot" \
-d u-boot-dtb.bin ${UIMAGE}

# Sign the uimage
echo dummy > dummy.txt
vbutil_kernel \
--pack ${KPART} \
--keyblock /usr/share/vboot/devkeys/kernel.keyblock \
--signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk \
--version 1 \
--vmlinuz ${UIMAGE} \
--bootloader dummy.txt \
--config dummy.txt \
--arch arm

# clean unneeded uimage
rm -f ${UIMAGE} dummy.txt

echo " "
echo "Now run:"
echo "sudo dd if=${KPART} of=/dev/<sd_card_part_1>"

