#!/usr/bin/env python

import uasyncio as asyncio

class StateConnect:
    def __init__(self, lcd, state):
        self.lcd = lcd
        self.state = state

        self.lock = asyncio.Lock()

    def enter(self, val = None):
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
            client = await d.device.connect()
        else:
            await d[0].disconnect()
            client = None

        async with self.lock:
            self.done = True
            self.client = client

    async def draw(self):
        self.lcd.fill(self.lcd.black)

        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        self.lcd.text("Connecting to Bluetooth device", 0, 10, self.lcd.red)

        if self.state:
            self.lcd.text("Connecting...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)
        else:
            self.lcd.text("Disconnecting...", 0, int(self.lcd.height / 2) - 5, self.lcd.white)

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
                    return 2 # selection
                else:
                    return 0 # scan

        self.lcd.show()
        return -1 # stay in this state
