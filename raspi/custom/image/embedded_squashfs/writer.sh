#!/bin/bash

set -u

# Copy the firmware image set into the device fs so it'll be available.
rsync -av ${OUTPUTPATH}/filesystem.squashfs ${IMAGEMOUNTPATH}/
