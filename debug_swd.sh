#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

echo Starting OpenOCD in background
echo "\n\nstarting new openocd" >> openocd.log
./build_debug/openocd/src/openocd -s build_debug/openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "cmsis_dap_vid_pid 0x2e8a 0x000c" >> openocd.log 2>&1 &
OPENOCD_PID=$!

# give OpenOCD some time to output stuff
sleep 1

echo -n Waiting for debugger to appear
while ! netstat -tna | grep 'LISTEN\>' | grep -q ':3333\>'; do
    echo -n .
    sleep 1
done

echo Starting GDB
arm-none-eabi-gdb \
-ex "set history save" \
-ex "show print pretty" \
-ex "target extended-remote localhost:3333" \
-ex "tui new-layout default src 1 status 1 cmd 2" \
-ex "tui layout default" \
-ex "tui enable" \
$1

echo Killing OpenOCD instance in background
kill $OPENOCD_PID
