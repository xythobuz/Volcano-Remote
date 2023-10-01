#!/usr/bin/env python

import uasyncio as asyncio
from poll import set_target_temp, get_current_temp

def draw_graph(lcd, min, val, max):
    if max == min:
        lcd.text("{} -> {} -> {}".format(min, val, max), 0, 100, lcd.white)
        return

    w = lcd.width - 10
    ratio = (val - min) / (max - min)
    wfull = int(w * ratio)
    wempty = w - wfull
    lcd.rect(4, 100, wfull + 1, 50, lcd.green, True)
    lcd.rect(4 + wfull, 100, wempty + 2, 50, lcd.green, False)
    lcd.text("{}".format(val), int(lcd.width / 2), 125, lcd.white)

class StateWaitTemp:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.value = val
        self.poller = asyncio.create_task(self.poll())
        self.temp = 0.0
        self.min = 0.0
        self.max = 100.0

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
        print("initial temp: {}".format(temp))
        async with self.lock:
            self.temp = temp
            self.min = temp

        print("Setting temperature: {}".format(self.max))
        await set_target_temp(device, self.max)

        while temp < self.max:
            temp = await get_current_temp(device)
            print("now at {}".format(temp))
            async with self.lock:
                self.temp = temp

    async def draw(self):
        self.lcd.fill(self.lcd.black)

        device, workflow, index = self.value
        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        self.lcd.text("Running Workflow - Heat {}".format(workflow["steps"][index][0]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            print("user abort")
            return 4 # heat off

        async with self.lock:
            if self.temp == 0.0:
                self.lcd.text("Setting temperature...", 0, 100, self.lcd.white)
            else:
                draw_graph(self.lcd, self.min, self.temp, self.max)

            if self.temp >= self.max:
                print("switch, {} >= {}".format(self.temp, self.max))
                return 7 # wait for time

        self.lcd.show()
        return -1 # stay in this state
