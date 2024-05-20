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

import uasyncio as asyncio
from poll import set_target_temp, get_current_temp
import math

# https://github.com/pimoroni/pimoroni-pico
def from_hsv(h, s, v):
    i = math.floor(h * 6.0)
    f = h * 6.0 - i
    v *= 255.0
    p = v * (1.0 - s)
    q = v * (1.0 - f * s)
    t = v * (1.0 - (1.0 - f) * s)

    i = int(i) % 6
    if i == 0:
        return int(v), int(t), int(p)
    if i == 1:
        return int(q), int(v), int(p)
    if i == 2:
        return int(p), int(v), int(t)
    if i == 3:
        return int(p), int(q), int(v)
    if i == 4:
        return int(t), int(p), int(v)
    if i == 5:
        return int(v), int(p), int(q)

# https://stackoverflow.com/a/1969274
def translate(value, leftMin, leftMax, rightMin, rightMax):
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin
    valueScaled = float(value - leftMin) / float(leftSpan)
    return rightMin + (valueScaled * rightSpan)

def draw_graph(lcd, min, val, max):
    if max == min:
        lcd.textC("{} -> {} -> {}".format(min, val, max), int(self.lcd.width / 2), int(lcd.height / 2), lcd.white)
        return

    hue = translate(val, min, max, 0.0, 0.333)
    r, g, b = from_hsv(hue, 1.0, 1.0)
    c = lcd.color(r, g, b)

    ratio = (val - min) / (max - min)
    lcd.pie(lcd.width / 2, lcd.height / 2, lcd.width - 42, lcd.white, c, ratio)

    lcd.textC("{} / {}".format(val, max), int(lcd.width / 2), int(lcd.height / 2), lcd.white, lcd.black)

class StateWaitTemp:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.temp = 0.0
        self.min = 0.0
        self.max = 100.0
        self.poller = asyncio.create_task(self.poll())

    def exit(self):
        self.poller.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], self.value[2])

    async def poll(self):
        device, workflow, index = self.value
        async with self.lock:
            self.temp = 0.0
            self.min = 0.0
            self.max = workflow["steps"][index][0]

        temp = await get_current_temp(device)
        async with self.lock:
            self.temp = temp
            self.min = temp

        await set_target_temp(device, self.max)

        while temp < self.max:
            temp = await get_current_temp(device)
            async with self.lock:
                self.temp = temp

    async def draw(self):
        device, workflow, index = self.value
        self.lcd.text("Running Workflow - Heat {}".format(workflow["steps"][index][0]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            return 4

        async with self.lock:
            if self.temp == 0.0:
                self.lcd.textC("Setting temperature...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
            else:
                draw_graph(self.lcd, self.min, self.temp, self.max)

            if self.temp >= self.max:
                return 7

        return -1
