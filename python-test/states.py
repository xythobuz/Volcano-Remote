#!/usr/bin/env python

import uasyncio as asyncio
from lcd import LCD
from state_scan import StateScan

class States:
    def __init__(self):
        self.states = []
        self.current = None

    def add(self, s):
        self.states.append(s)

    def run(self):
        if self.current == None:
            self.current = 0
            self.states[self.current].enter()

        next = asyncio.run(self.states[self.current].draw())
        if next >= 0:
            self.states[self.current].exit()
            self.current = next
            self.states[self.current].enter()

if True:#__name__ == "__main__":
    lcd = LCD()
    lcd.brightness(1.0)

    states = States()

    # 0 - Scan
    scan = StateScan(lcd)
    states.add(scan)

    while True:
        states.run()
