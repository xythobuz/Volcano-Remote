#!/usr/bin/env python

import time
import uasyncio as asyncio
from poll import set_state
from state_wait_temp import draw_graph

class StateNotify:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.notifier = asyncio.create_task(self.notify())
        self.done = False

    def exit(self):
        self.notifier.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], self.value[2])

    async def notify(self):
        device, workflow, index = self.value
        count, duration = workflow["notify"]

        for i in range(0, count):
            print("Turning on pump")
            await set_state(device, (None, True))
            await asyncio.sleep_ms(int(duration * 1000))

            print("Turning off pump")
            await set_state(device, (None, False))
            await asyncio.sleep_ms(int(duration * 1000))

        async with self.lock:
            self.done = True

    async def draw(self):
        self.lcd.text("Running Workflow - Notify", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            print("user abort")
            return 4 # heat off

        async with self.lock:
            if self.done:
                return 4 # heater off

        return -1 # stay in this state
