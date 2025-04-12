#!/usr/bin/env bash

# Needed to function on AMD64 arch CPUs.
sudo su -c "mount binfmt_misc -t binfmt_misc /proc/sys/fs/binfmt_misc && echo 1 > /proc/sys/fs/binfmt_misc/status"

echo "Updating deps..."
sudo apt-get update
sudo rpi-image-gen/install_deps.sh

echo "Building image..."
~/rpi-image-gen/build.sh -D ~/custom -c sacnlogger "$@"
