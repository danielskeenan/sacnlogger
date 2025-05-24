#!/bin/sh

set -eu

rootfs=$1
genimg_in=$2

sed -i "s|root=\([^ ]*\)|root=\/dev\/disk\/by-label\/BOOT rootflags=ro,umask=022 ro|" ${rootfs}/boot/firmware/cmdline.txt

cat << EOF > ${rootfs}/etc/fstab
/dev/disk/by-label/BOOT /boot/firmware  vfat ro,noatime,fmask=0022,dmask=0022,codepage=437,iocharset=ascii,shortname=mixed,errors=remount-ro 0 2
EOF


cp ro_assets.cfg.in ${genimg_in}/genimage01.cfg

# Because the compressed filesystem lives here, genimage can't guess the required size.
# Keep an eye on space usage and adjust this as needed.
# TODO: Is there a way to automate this?

WRITER=$(readlink -f writer.sh)

cat main.cfg.in | sed \
   -e "s|<IMAGE_DIR>|$IGconf_sys_outputdir|g" \
   -e "s|<IMAGE_NAME>|$IGconf_image_name|g" \
   -e "s|<IMAGE_SUFFIX>|$IGconf_image_suffix|g" \
   -e "s|<BOOT_PART_SIZE>|$IGconf_image_boot_part_size|g" \
   -e "s|<SECTOR_SIZE>|$IGconf_device_sector_size|g" \
   -e "s|<EMBED_HOOK>|$WRITER|g" \
   > ${genimg_in}/genimage02.cfg
