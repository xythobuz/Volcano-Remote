#!/usr/bin/env python

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
                        self.lcd.text("Turning heater off...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
                    else:
                        self.lcd.text("Resetting temperature...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
                else:
                    self.lcd.text("Turning heater on...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)

        return -1
