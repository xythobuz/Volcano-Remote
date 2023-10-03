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
        self.pumper = asyncio.create_task(self.pump())
        self.done = False

        device, workflow, index = self.value
        self.start = None
        self.duration = workflow["steps"][index][2]

    def exit(self):
        self.pumper.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], self.value[2] + 1)

    async def pump(self):
        device, workflow, index = self.value

        print("Turning on pump")
        await set_state(device, (None, True))
        async with self.lock:
            self.start = time.time()

        await asyncio.sleep_ms(int(self.duration * 1000))

        print("Turning off pump")
        await set_state(device, (None, False))
        async with self.lock:
            self.done = True

    async def draw(self):
        device, workflow, index = self.value
        self.lcd.text("Running Workflow - Pump {}".format(workflow["steps"][index][2]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            print("user abort")
            return 4 # heat off

        async with self.lock:
            if self.start != None:
                now = time.time()
                if now - self.start <= self.duration:
                    draw_graph(self.lcd, 0.0, now - self.start, self.duration)
                else:
                    self.lcd.text("Turning off pump...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
            else:
                self.lcd.text("Turning on pump...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)

            if self.done:
                if self.value[2] >= (len(workflow["steps"]) - 1):
                    if workflow["notify"] != None:
                        return 9 # notify
                    else:
                        return 4 # heater off
                else:
                    return 6 # wait for temperature

        return -1 # stay in this state
