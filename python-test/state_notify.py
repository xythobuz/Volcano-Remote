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
        self.done = False
        self.step = 0
        self.max = 0
        self.notifier = asyncio.create_task(self.notify())

    def exit(self):
        self.notifier.cancel()

        if self.lock.locked():
            self.lock.release()

        return (self.value[0], self.value[1], self.value[2])

    async def notify(self):
        device, workflow, index = self.value
        count, duration = workflow["notify"]

        async with self.lock:
            self.max = count * 2

        for i in range(0, count):
            await asyncio.sleep_ms(int(duration * 1000))
            await set_state(device, (None, True))
            async with self.lock:
                self.step += 1

            await asyncio.sleep_ms(int(duration * 1000))
            await set_state(device, (None, False))
            async with self.lock:
                self.step += 1

        async with self.lock:
            self.done = True

    async def draw(self):
        self.lcd.text("Running Workflow - Notify", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            return 4

        async with self.lock:
            draw_graph(self.lcd, 0, self.step, self.max)

            if self.done:
                return 4

        return -1
