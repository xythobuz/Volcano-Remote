#!/bin/bash

if [ $# -ne 0 ] ; then
cat << EOF | rshell
cp flow.py /pyboard
cp poll.py /pyboard
cp scan.py /pyboard
cp lcd.py /pyboard
cp states.py /pyboard
cp state_scan.py /pyboard
cp $1 /pyboard/main.py
EOF
else
cat << EOF | rshell
cp flow.py /pyboard
cp poll.py /pyboard
cp scan.py /pyboard
cp lcd.py /pyboard
cp states.py /pyboard
cp state_scan.py /pyboard
EOF
fi
