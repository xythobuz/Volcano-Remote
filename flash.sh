#!/bin/bash

# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

set -euo pipefail

SERIAL=/dev/serial/by-id/usb-xythobuz_VolcanoRC_*
DISK=/dev/disk/by-label/RPI-RP2
MOUNT=/mnt/tmp

if [ ! -e $DISK ]
then
    echo Resetting Raspberry Pi Pico
    echo -n -e "\\x18" > $SERIAL
fi

echo -n Waiting for disk to appear
until [ -e $DISK ]
do
    echo -n .
    sleep 1
done
echo

echo Mounting bootloader disk
sudo mount $DISK $MOUNT

echo Copying binary
sudo cp $1 $MOUNT

echo Done
