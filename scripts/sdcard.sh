#!/bin/sh -e

DEVICE="$1"

if [ ! -b "$DEVICE" ]; then
echo "Usage: $0 <device>"
  exit 1
fi

if [ "`id -u`" -ne 0 ]; then
echo "Please run this program as root."
fi

echo "I'm about to overwrite ${DEVICE}. All existing data will be lost."
until [ "$REPLY" = y -o "$REPLY" = yes ] || [ "$REPLY" = n -o "$REPLY" = no ]; do
read -p "Do you want to continue? " REPLY
done
[ "$REPLY" = n -o "$REPLY" = no ] && exit

grep -q $DEVICE /proc/mounts && grep $DEVICE /proc/mounts | cut -f2 -d\ | xargs umount
dd if=/dev/zero of=$DEVICE bs=1M count=10

parted -s $DEVICE mktable gpt
sgdisk -n 1:0:+16M -t 1:7f00 -n 2:0:+16M -t 2:7f00 -n 3:0:+512M -n 4:0:0 -p $DEVICE

echo $DEVICE | grep -q mmc && OPT=p
boot_part=${DEVICE}${OPT}3
root_part=${DEVICE}${OPT}4

mkfs.ext2 -L BOOT $boot_part
mkfs.ext4 -L ROOT $root_part

cgpt add -S 1 -T 5 -P 12 -i 1 ${DEVICE}${OPT}
dd if=nv_uboot-snow.kpart of=${DEVICE}${OPT}1

sync
