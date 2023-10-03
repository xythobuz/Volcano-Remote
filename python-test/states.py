#!/usr/bin/env python

import uasyncio as asyncio
from lcd import LCD
from state_scan import StateScan
from state_connect import StateConnect
from state_select import StateSelect
from state_heat import StateHeat
from state_wait_temp import StateWaitTemp
from state_wait_time import StateWaitTime
from state_pump import StatePump
from state_notify import StateNotify

class States:
    def __init__(self, lcd):
        self.lcd = lcd
        self.states = []
        self.current = None

    def add(self, s):
        self.states.append(s)

    async def draw(self):
        self.lcd.fill(self.lcd.black)
        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        r = await self.states[self.current].draw()
        self.lcd.show()
        return r

    def run(self):
        if self.current == None:
            self.current = 0
            self.states[self.current].enter()

        next = asyncio.run(self.draw())
        if next >= 0:
            print("switch to {}".format(next))
            val = self.states[self.current].exit()
            self.current = next
            self.states[self.current].enter(val)

if True:#__name__ == "__main__":
    lcd = LCD()
    lcd.brightness(1.0)

    states = States(lcd)

    # 0 - Scan
    # passes ScanResult to 2, select
    scan = StateScan(lcd)
    states.add(scan)

    # 1 - Connect
    # passes device and selected workflow to 3, heater on
    conn = StateConnect(lcd, True)
    states.add(conn)

    # 2 - Select
    # passes ScanResult and selected workflow to 1, connect
    select = StateSelect(lcd)
    states.add(select)

    # 3 - Heater On
    heatOn = StateHeat(lcd, True)
    states.add(heatOn)

    # 4 - Heater Off
    heatOff = StateHeat(lcd, False)
    states.add(heatOff)

    # 5 - Disconnect
    disconn = StateConnect(lcd, False)
    states.add(disconn)

    # 6 - Wait for temperature
    waitTemp = StateWaitTemp(lcd)
    states.add(waitTemp)

    # 7 - Wait for time
    waitTime = StateWaitTime(lcd)
    states.add(waitTime)

    # 8 - Pump
    pump = StatePump(lcd)
    states.add(pump)

    # 9 - Notify
    notify = StateNotify(lcd)
    states.add(notify)

    while True:
        states.run()
