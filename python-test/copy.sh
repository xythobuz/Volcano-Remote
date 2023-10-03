#!/bin/bash

if [ $# -ne 0 ] ; then
cat << EOF | rshell
cp poll.py /pyboard
cp scan.py /pyboard
cp lcd.py /pyboard
cp workflows.py /pyboard
cp states.py /pyboard
cp state_scan.py /pyboard
cp state_connect.py /pyboard
cp state_select.py /pyboard
cp state_heat.py /pyboard
cp state_wait_temp.py /pyboard
cp state_wait_time.py /pyboard
cp state_pump.py /pyboard
cp state_notify.py /pyboard
cp $1 /pyboard/main.py
EOF
else
cat << EOF | rshell
cp poll.py /pyboard
cp scan.py /pyboard
cp lcd.py /pyboard
cp workflows.py /pyboard
cp states.py /pyboard
cp state_scan.py /pyboard
cp state_connect.py /pyboard
cp state_select.py /pyboard
cp state_heat.py /pyboard
cp state_wait_temp.py /pyboard
cp state_wait_time.py /pyboard
cp state_pump.py /pyboard
cp state_notify.py /pyboard
rm /pyboard/main.py
EOF
fi
