#!/bin/sh
# Mount the root filesystem.
# Based on https://unix.stackexchange.com/a/445141, added error handling and modified for initramfs-tools and
# Raspberry Pi.

PREREQ=""
prereqs()
{
	echo "$PREREQ"
}
case $1 in
prereqs)
	prereqs
	exit 0
	;;
esac
. /scripts/functions
# Begin real processing below this line

# load modules
modprobe overlay
if [ $? -ne 0 ]; then
    panic "ERROR: missing overlay kernel module"
fi
modprobe squashfs
if [ $? -ne 0 ]; then
    panic "ERROR: missing squashfs kernel module"
fi

log_begin_msg "Mounting root filesystem overlay..."

# create some temporary directories under the initramfs's /run
# they will be our mountpoints and such, which will get moved
# by the default script to the actual root filesystem...
mkdir -m 755 /run/rootfs
mount -t tmpfs -o size=90%,mode=755,suid,exec tmpfs /run/rootfs
if [ $? -ne 0 ]; then
    panic "ERROR: failed to mount tmpfs"
fi
mkdir -m 755 /run/rootfs/drive /run/rootfs/ro /run/rootfs/rw /run/rootfs/.workdir

# move the original root that was mounted, temporarily
mount -n -o move "${rootmnt}" /run/rootfs/drive
if [ $? -ne 0 ]; then
    panic "ERROR: failed to relocate original root"
fi

# mount the squashfs and then the overlay to our designated locations
# The mount command available here doesn't know how to create loopback devices, so need to do it separately.
losetup /dev/loop0 /run/rootfs/drive/filesystem.squashfs
if [ $? -ne 0 ]; then
    panic "ERROR: failed to mount compressed filesystem loopback"
fi
mount -t squashfs -o ro /dev/loop0 /run/rootfs/ro
if [ $? -ne 0 ]; then
    panic "ERROR: failed to mount compressed filesystem directory"
fi
# Now we have both the real filesystem and the tmpfs writable layer, create the overlay.
mount -t overlay -o lowerdir=/run/rootfs/ro,upperdir=/run/rootfs/rw,workdir=/run/rootfs/.workdir root "${rootmnt}"
if [ $? -ne 0 ]; then
    panic "ERROR: failed to mount overlay"
fi
# At this point we have our overlay root at ${rootmnt}!

# Move the drive's filesystem mount to where RPI's firmware partition should be mounted.
mkdir -p "${rootmnt}/boot/firmware"
mount -n -o move /run/rootfs/drive "${rootmnt}/boot/firmware"
if [ $? -ne 0 ]; then
    panic "ERROR: failed to relocate firmware partition"
fi
# Cleanup the scratch dir.
rm -d /run/rootfs/drive
log_end_msg
