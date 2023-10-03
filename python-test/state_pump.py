#!/usr/bin/env python

import time
import uasyncio as asyncio
from poll import set_state
from state_wait_temp import draw_graph

class StatePump:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.done = False

        device, workflow, index = self.value
        self.start = None
        self.duration = workflow["steps"][index][2]

        self.pumper = asyncio.create_task(self.pump())

    def exit(self):
        self.pumper.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], self.value[2] + 1)

    async def pump(self):
        device, workflow, index = self.value

        await set_state(device, (None, True))
        async with self.lock:
            self.start = time.time()

        await asyncio.sleep_ms(int(self.duration * 1000))

        await set_state(device, (None, False))
        async with self.lock:
            self.done = True

    async def draw(self):
        device, workflow, index = self.value
        self.lcd.text("Running Workflow - Pump {}".format(workflow["steps"][index][2]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            return 4

        async with self.lock:
            if self.start != None:
                now = time.time()
                if now - self.start <= self.duration:
                    draw_graph(self.lcd, 0.0, now - self.start, self.duration)
                else:
                    self.lcd.textC("Turning off pump...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
            else:
                self.lcd.textC("Turning on pump...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)

            if self.done:
                if self.value[2] >= (len(workflow["steps"]) - 1):
                    if workflow["notify"] != None:
                        return 9
                    else:
                        return 4
                else:
                    return 6

        return -1
