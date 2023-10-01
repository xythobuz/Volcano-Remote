#!/usr/bin/env python

import uasyncio as asyncio
from poll import set_state

class StateHeat:
    def __init__(self, lcd, state):
        self.lcd = lcd
        self.state = state

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.heater = asyncio.create_task(self.heat())
        self.done = False

    def exit(self):
        self.heater.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], 0)

    async def heat(self):
        print("Setting heater: {}".format(self.state))
        pump = None
        if self.state == False:
            pump = False
        await set_state(self.value[0], (self.state, pump))

        async with self.lock:
            self.done = True

    async def draw(self):
        self.lcd.fill(self.lcd.black)

        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        self.lcd.text("Running Workflow - Heat {}".format(self.state), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            print("user abort")
            if self.state:
                async with self.lock:
                    if self.done:
                        return 4 # heat off
            else:
                return 5 # disconnect

        async with self.lock:
            if self.done:
                if self.state == False:
                    return 5 # disconnect
                else:
                    return 6 # wait for temperature

        self.lcd.show()
        return -1 # stay in this state
