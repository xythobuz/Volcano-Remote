#!/usr/bin/env python

import uasyncio as asyncio
from scan import ble_scan

class StateScan:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        self.results = []
        self.current = None
        self.scanner = asyncio.create_task(self.scan())

    def exit(self):
        self.scanner.cancel()

        if self.lock.locked():
            self.lock.release()

        return self.results[self.current]

    async def scan(self):
        while True:
            new = await ble_scan(None, None, 0.25)

            for n in new:
                name = n.name()
                mac = n.device.addr_hex()
                rssi = n.rssi
                value = [name, mac, rssi]

                async with self.lock:
                    found = False
                    for i in range(0, len(self.results)):
                        if self.results[i][1] == mac:
                            found = True
                            self.results[i][0] = name
                            self.results[i][2] = rssi
                            break

                    if found == False:
                        self.results.append(value)

    def draw_list(self):
        for i, d in enumerate(self.results):
            name, mac, rssi = self.results[i]
            s1 = "{}: {}".format(i + 1, name)
            s2 = "[{}] {}".format(mac, rssi)

            off = i * 25 + 30
            if off >= self.lcd.height:
                break

            c1 = self.lcd.white
            if name == "S&B VOLCANO H":
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
        self.lcd.text("Scanning for Bluetooth devices", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        async with self.lock:
            if keys.once("enter"):
                if self.current < len(self.results):
                    return 2
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

        return -1
