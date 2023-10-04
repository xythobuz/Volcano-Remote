#!/usr/bin/env python

import uasyncio as asyncio
from poll import cache_services_characteristics
import machine

class StateConnect:
    def __init__(self, lcd, state):
        self.lcd = lcd
        self.state = state

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.step = False
        self.iteration = 0.0
        self.done = False
        self.client = None
        self.connector = asyncio.create_task(self.connect(val))

    def exit(self):
        self.connector.cancel()

        if self.lock.locked():
            self.lock.release()

        if self.state == False:
            machine.soft_reset()

        return self.client

    async def progress(self, n):
        async with self.lock:
            self.iteration = n

    async def connect(self, d):
        async with self.lock:
            self.done = False

        if self.state:
            client = await d[0].device.connect()
            async with self.lock:
                self.step = True

            await cache_services_characteristics(client, self.progress)
        else:
            await d[0].disconnect()
            client = None

        async with self.lock:
            self.done = True
            self.client = (client, d[1])

    async def draw(self):
        self.lcd.text("Connecting to Bluetooth device", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            if self.state:
                return 5
            else:
                return 0

        async with self.lock:
            if self.done:
                if self.state:
                    return 3
                else:
                    return 0
            else:
                if self.state == False:
                    self.lcd.textC("Disconnecting...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
                else:
                    if self.step == False:
                        self.lcd.textC("Connecting...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
                    else:
                        self.lcd.pie(self.lcd.width / 2, self.lcd.height / 2, self.lcd.width - 30, self.lcd.red, self.lcd.green, self.iteration)
                        self.lcd.textC("Fetching parameters...", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)

        return -1
