#!/usr/bin/env python

import uasyncio as asyncio
from scan import ble_scan
from flow import flow

class StateScan:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self):
        self.scanner = asyncio.create_task(self.scan())
        self.results = []
        self.current = None

    def exit(self):
        self.scanner.cancel()

        if self.lock.locked():
            self.lock.release()

    async def scan(self):
        while True:
            new = await ble_scan(None, None, 0.25)

            async with self.lock:
                for n in new:
                    found = False
                    for r in self.results:
                        if r.device.addr_hex() == n.device.addr_hex():
                            found = True
                            r = n
                            break
                    if not found:
                        self.results.append(n)

    def draw_list(self):
        for i, d in enumerate(self.results):
            s1 = "{}".format(d.name())
            s2 = "{}: [{}] {}".format(i + 1, d.device.addr_hex(), d.rssi)

            off = i * 25 + 30
            if off >= self.lcd.height:
                break

            c1 = self.lcd.white
            if s1 == "S&B VOLCANO H":
                c1 = self.lcd.green
                if self.current == None:
                    self.current = i
            elif self.current == i:
                c1 = self.lcd.red
            c2 = self.lcd.white
            if self.current == i:
                c2 = self.lcd.red

            self.lcd.hline(0, off, self.lcd.width, self.lcd.blue)
            self.lcd.text(s1, 0, off + 2, c1)
            self.lcd.text(s2, 0, off + 12, c2)

    async def draw(self):
        self.lcd.fill(self.lcd.black)

        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        self.lcd.text("Scanning for Bluetooth devices", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        async with self.lock:
            if keys.once("enter"):
                if self.current < len(self.results):
                    #return 1 # connect
                    client = await self.results[self.current].device.connect()
                    await flow(client)
            elif keys.once("up"):
                if self.current == None:
                    self.current = len(self.results) - 1
                elif self.current > 0:
                    self.current -= 1
            elif keys.once("down"):
                if self.current == None:
                    self.current = 0
                elif self.current < (len(self.results) - 1):
                    self.current += 1

            self.draw_list()

        self.lcd.show()
        return -1 # stay in this state
