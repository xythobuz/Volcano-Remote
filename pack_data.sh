#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"
echo "Packing data"

mkdir -p build/src
cp COPYING build/src
cp README.md build/src
cp CMakeLists.txt build/src
cp -r include build/src
cp -r src build/src

cd build
rm -rf data.tar data.tar.xz
tar -f data.tar -c src
xz -z -9 data.tar

xxd -i data.tar.xz > pack_data.h
sed -i 's/unsigned/static const unsigned/g' pack_data.h
