#!/bin/bash

# ----------------------------------------------------------------------------
# Copyright (c) 2023 Kauzerei (openautolab@kauzerei.de)
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

# space separated list of scad files (without extension)
MODULES="case"
OUTDIR="stl"

# enter directory of script (case)
cd "$(dirname "$0")"

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Mac OS X detected"
    SCAD="open -n -a OpenSCAD --args"
else
    echo "Linux detected"
    SCAD="openscad"
fi

echo "deleting previous build output"
rm -rf $OUTDIR
mkdir -p $OUTDIR

for MODULE in $MODULES
do
    PARTS=$(grep -o "part.*//.*\[.*]" ${MODULE}.scad | sed 's/,/ /g' | sed 's/.*\[\([^]]*\)\].*/\1/g')
    echo "generating from ${MODULE}"

    for PART in ${PARTS}
    do
        if [[ "${PART}" != "OPT_"* ]]; then
            echo ${PART}
            FILENAME=$(echo $OUTDIR/${MODULE}_${PART}.stl | tr '[:upper:]' '[:lower:]')
            $SCAD $(pwd)/${MODULE}.scad --D part=\"${PART}\" --o $(pwd)/${FILENAME}
        fi
    done
done
