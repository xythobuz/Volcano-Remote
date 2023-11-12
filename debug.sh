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

echo -n Waiting for serial port to appear
until [ -e $SERIAL ]
do
    echo -n .
    sleep 1
done
echo

echo Opening picocom terminal
echo "[C-a] [C-x] to exit"
echo

picocom -q --omap crcrlf --imap lfcrlf $SERIAL
