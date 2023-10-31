#!/usr/bin/env python3

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

from lcd import LCD
lcd = LCD()

import uasyncio as asyncio
import io
import sys
import machine
import os
import gc
import time
from state_wait_temp import from_hsv, translate

# https://github.com/pimoroni/pimoroni-pico/blob/main/micropython/examples/pico_lipo_shim/battery_pico.py
# https://github.com/pimoroni/enviro/pull/146
# TODO https://github.com/micropython/micropython/issues/11185

full_battery = 4.1
empty_battery = 3.2
batt_warn_limit = 15
batt_reread_limit = 2.7

charging = machine.Pin("WL_GPIO2", machine.Pin.IN)
conversion_factor = 3 * 3.3 / 65535
cachedVoltage = None
lastCaching = time.time()

def set_pad(gpio, value):
    machine.mem32[0x4001c000 | (4 + (4 * gpio))] = value

def get_pad(gpio):
    return machine.mem32[0x4001c000 | (4 + (4 * gpio))]

def batteryVoltageRead():
    vsys = machine.ADC(3)
    voltage = vsys.read_u16() * conversion_factor
    return voltage

def batteryVoltageAverage():
    old_pad = get_pad(29)
    set_pad(29, 128)  # no pulls, no output, no input

    sample_count = 3
    voltage = 0
    for i in range(0, sample_count):
        voltage += batteryVoltageRead()
    voltage /= sample_count

    set_pad(29, old_pad)
    return voltage

def batteryVoltage():
    global cachedVoltage, lastCaching

    if ((time.time() - lastCaching) > 0) or (cachedVoltage == None):
        lastCaching = time.time()
        cachedVoltage = batteryVoltageAverage()
        if cachedVoltage <= batt_reread_limit:
            cachedVoltage = batteryVoltageAverage()

    percentage = 100.0 * ((cachedVoltage - empty_battery) / (full_battery - empty_battery))
    if percentage >= 100.0:
        percentage = 99.0
    elif percentage < 0.0:
        percentage = 0.0

    return cachedVoltage, percentage

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

        ret = await self.states[self.current].draw()

        voltage, percentage = batteryVoltage()
        s = "Charging"
        c = self.lcd.white
        if charging.value() != 1:
            s = "{:.0f}% ({:.2f}V)".format(percentage, voltage)
            if percentage <= batt_warn_limit:
                c = self.lcd.red
            else:
                hue = translate(percentage, batt_warn_limit, 100, 0.0, 0.333)
                r, g, b = from_hsv(hue, 1.0, 1.0)
                c = self.lcd.color(r, g, b)
        whole = "Batt: {}".format(s)
        self.lcd.text(whole, 0, self.lcd.height - 10, c)

        off = (len(whole) + 1) * 8
        if percentage <= batt_warn_limit:
            self.lcd.text("CHARGE NOW!", off, self.lcd.height - 10, self.lcd.red)
        elif charging.value() != 1:
            self.lcd.rect(off, self.lcd.height - 10, self.lcd.width - off, 8, c, False)
            max_w = self.lcd.width - off - 2
            w = int(percentage / 100.0 * max_w)
            self.lcd.rect(off + 1, self.lcd.height - 9, w, 6, c, True)

        self.lcd.show()
        return ret

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

def main():
    # splash screen
    from state_wait_temp import from_hsv
    for x in range(0, lcd.width):
        hue = x / (lcd.width - 1)
        r, g, b = from_hsv(hue, 1.0, 1.0)
        c = lcd.color(r, g, b)
        lcd.rect(x, 0, 1, lcd.height, c)

    lcd.textC("S&B Volcano Remote", int(lcd.width / 2), 10, lcd.green, lcd.black)
    lcd.textC("by xythobuz",        int(lcd.width / 2), 20, lcd.yellow, lcd.black)
    lcd.textC("Initializing...",    int(lcd.width / 2), 30, lcd.white, lcd.black)

    import _git
    lcd.textC(_git.git_branch, int(lcd.width / 2), int(lcd.height / 2) - 10, lcd.green, lcd.black)
    lcd.textC(_git.git_hash, int(lcd.width / 2), int(lcd.height / 2), lcd.yellow, lcd.black)
    lcd.textC(_git.build_date, int(lcd.width / 2), int(lcd.height / 2) + 10, lcd.white, lcd.black)

    lcd.textC(os.uname()[0][ 0 : 30], int(lcd.width / 2), lcd.height - 50, lcd.green, lcd.black)
    lcd.textC(os.uname()[3][ 0 : 30], int(lcd.width / 2), lcd.height - 40, lcd.yellow, lcd.black)
    lcd.textC(os.uname()[3][30 : 60], int(lcd.width / 2), lcd.height - 30, lcd.yellow, lcd.black)
    lcd.textC(os.uname()[4][ 0 : 30], int(lcd.width / 2), lcd.height - 20, lcd.white, lcd.black)
    lcd.textC(os.uname()[4][30 : 60], int(lcd.width / 2), lcd.height - 10, lcd.white, lcd.black)

    lcd.show()

    # bootloader access with face buttons
    keys = lcd.buttons()
    if keys.once("a") and keys.once("b"):
        machine.bootloader()

    state_machine(lcd)

try:
    main()
except Exception as e:
    sys.print_exception(e)

    gc.collect()
    os = io.StringIO()
    sys.print_exception(e, os)
    s = os.getvalue()
    os.close()

    lcd.fill(lcd.black)
    lcd.textBlock(s, lcd.white)
    lcd.show()

    raise e
