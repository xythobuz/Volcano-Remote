#!/usr/bin/env python

import uasyncio as asyncio
from poll import set_target_temp, get_current_temp

def draw_graph(lcd, min, val, max):
    if max == min:
        lcd.text("{} -> {} -> {}".format(min, val, max), 0, int(lcd.height / 2) - 5, lcd.white)
        return

    ratio = (val - min) / (max - min)
    lcd.pie(lcd.width / 2, lcd.height / 2, lcd.width - 30, lcd.red, lcd.green, ratio)

    lcd.text("{} / {}".format(val, max), int(lcd.width / 2), int(lcd.height / 2) - 5, lcd.white)

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
                self.lcd.text("Setting temperature...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
            else:
                draw_graph(self.lcd, self.min, self.temp, self.max)

            if self.temp >= self.max:
                return 7

        return -1
