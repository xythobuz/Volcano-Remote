#!/usr/bin/env python

import uasyncio as asyncio
from poll import cache_services_characteristics

class StateConnect:
    def __init__(self, lcd, state):
        self.lcd = lcd
        self.state = state

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.step = False
        self.done = False
        self.client = None
        self.connector = asyncio.create_task(self.connect(val))

    def exit(self):
        self.connector.cancel()

        if self.lock.locked():
            self.lock.release()

        return self.client

    async def connect(self, d):
        async with self.lock:
            self.done = False

        if self.state:
            client = await d[0].device.connect()
            async with self.lock:
                self.step = True
            await cache_services_characteristics(client)
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
            print("user abort")
            if self.state:
                return 5 # disconnect
            else:
                return 0 # scan

        async with self.lock:
            if self.done:
                if self.state:
                    return 3 # heater on
                else:
                    return 0 # scan
            else:
                if self.state == False:
                    self.lcd.text("Disconnecting...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
                else:
                    if self.step == False:
                        self.lcd.text("Connecting...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
                    else:
                        self.lcd.text("Fetching parameters...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)

        return -1 # stay in this state
