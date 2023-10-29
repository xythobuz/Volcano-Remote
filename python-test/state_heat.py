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
from poll import set_state, set_target_temp

class StateHeat:
    def __init__(self, lcd, state):
        self.lcd = lcd
        self.state = state

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.done = False
        self.step = False
        self.heater = asyncio.create_task(self.heat())

    def exit(self):
        self.heater.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], 0)

    async def heat(self):
        pump = None
        if self.state == False:
            pump = False

        await set_state(self.value[0], (self.state, pump))

        if self.state == False:
            async with self.lock:
                self.step = True

            temp = self.value[1]["reset_temperature"]
            if temp != None:
                await set_target_temp(self.value[0], temp)

        async with self.lock:
            self.done = True

    async def draw(self):
        self.lcd.text("Running Workflow - Heat {}".format(self.state), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            if self.state:
                async with self.lock:
                    if self.done:
                        return 4
                    else:
                        return 5
            else:
                return 5

        async with self.lock:
            if self.done:
                if self.state == False:
                    return 5
                else:
                    return 6
            else:
                if self.state == False:
                    if self.state == False:
                        self.lcd.textC("Turning heater off...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
                    else:
                        self.lcd.textC("Resetting temperature...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
                else:
                    self.lcd.textC("Turning heater on...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)

        return -1
