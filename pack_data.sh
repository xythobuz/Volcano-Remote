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

# TODO take from config.h
DISK_BLOCK_SIZE=512
DISK_BLOCK_COUNT=384

cd "$(dirname "$0")"
echo "Packing data"

rm -rf $1/src
mkdir -p $1/src
cp COPYING $1/src
cp README.md $1/src
cp CMakeLists.txt $1/src
cp .gitmodules $1/src/gitmodules
cp -r include $1/src
cp -r conf $1/src
cp -r src $1/src

echo "Compressing data"
cd $1
rm -rf data.tar data.tar.xz
tar -f data.tar -c src
xz -z -9 data.tar

echo "Creating empty bin"
dd if=/dev/zero of=fat_fs.bin bs=$DISK_BLOCK_SIZE count=$DISK_BLOCK_COUNT

echo "Writing FAT file system"
mkfs.vfat fat_fs.bin

echo "Copying source code archive"
mcopy -i fat_fs.bin ../data/README.md ::README.md
mcopy -i fat_fs.bin data.tar.xz ::src.tar.xz

echo "Converting to object file"
arm-none-eabi-objcopy -I binary -O elf32-littlearm \
    --rename-section .data=.fat_fs_bin,CONTENTS,ALLOC,LOAD,READONLY,DATA \
    fat_fs.bin fat_fs.o
