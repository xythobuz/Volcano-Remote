#!/usr/bin/env python

import uasyncio as asyncio

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
            val = self.states[self.current].exit()
            self.current = next
            self.states[self.current].enter(val)

def state_machine(lcd):
    states = States(lcd)

    # 0 - Scan
    from state_scan import StateScan
    scan = StateScan(lcd)
    states.add(scan)

    # 1 - Connect
    from state_connect import StateConnect
    conn = StateConnect(lcd, True)
    states.add(conn)

    # 2 - Select
    from state_select import StateSelect
    select = StateSelect(lcd)
    states.add(select)

    # 3 - Heater On
    from state_heat import StateHeat
    heatOn = StateHeat(lcd, True)
    states.add(heatOn)

    # 4 - Heater Off
    heatOff = StateHeat(lcd, False)
    states.add(heatOff)

    # 5 - Disconnect
    disconn = StateConnect(lcd, False)
    states.add(disconn)

    # 6 - Wait for temperature
    from state_wait_temp import StateWaitTemp
    waitTemp = StateWaitTemp(lcd)
    states.add(waitTemp)

    # 7 - Wait for time
    from state_wait_time import StateWaitTime
    waitTime = StateWaitTime(lcd)
    states.add(waitTime)

    # 8 - Pump
    from state_pump import StatePump
    pump = StatePump(lcd)
    states.add(pump)

    # 9 - Notify
    from state_notify import StateNotify
    notify = StateNotify(lcd)
    states.add(notify)

    while True:
        states.run()

from lcd import LCD
lcd = LCD()
lcd.brightness(1.0)

try:
    state_machine(lcd)
except Exception as e:
    import io
    import sys
    os = io.StringIO()
    sys.print_exception(e, os)
    s = os.getvalue()
    os.close()

    lcd.fill(lcd.black)
    lcd.textBlock(s, lcd.white)
    lcd.show()
    raise e
