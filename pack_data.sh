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

cd "$(dirname "$0")"
echo "Packing data"

rm -rf build/src
mkdir -p build/src
cp COPYING build/src
cp README.md build/src
cp CMakeLists.txt build/src
cp .gitmodules build/src/gitmodules
cp -r include build/src
cp -r conf build/src
cp -r src build/src

cd build
rm -rf data.tar data.tar.xz
tar -f data.tar -c src
xz -z -9 data.tar

xxd -i data.tar.xz > pack_data.h
sed -i 's/unsigned/static const unsigned/g' pack_data.h
