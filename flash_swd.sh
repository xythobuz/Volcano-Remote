#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

./build_debug/openocd/src/openocd -s build_debug/openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "cmsis_dap_vid_pid 0x2e8a 0x000c" -c "program $1 verify reset exit"
