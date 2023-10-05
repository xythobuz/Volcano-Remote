#!/usr/bin/env python

import uasyncio as asyncio
from scan import ble_scan
import time

class StateScan:
    def __init__(self, lcd):
        self.lcd = lcd

        self.lock = asyncio.Lock()

    def enter(self, val = None):
        n = None
        self.results = []
        self.current = None
        self.menuOff = 0
        self.scanner = asyncio.create_task(self.scan())

    def exit(self):
        self.scanner.cancel()

        if self.lock.locked():
            self.lock.release()

        return self.results[self.current][4]

    async def scan(self):
        while True:
            new = await ble_scan(None, None, 0.25)

            for n in new:
                name = n.name()
                mac = n.device.addr_hex()
                rssi = n.rssi
                value = [name, mac, rssi, time.time(), n]

                async with self.lock:
                    found = False
                    for i in range(0, len(self.results)):
                        if self.results[i][1] == mac:
                            found = True
                            self.results[i][0] = name
                            self.results[i][2] = rssi
                            self.results[i][3] = time.time()
                            self.results[i][4] = n
                            break

                    if found == False:
                        self.results.append(value)

    def draw_list(self):
        if len(self.results) <= 0:
            self.lcd.textC("No devices found yet", int(self.lcd.width / 2), int(self.lcd.height / 2), self.lcd.white)
            return

        for i, d in enumerate(self.results):
            if i < self.menuOff:
                continue

            off = (i - self.menuOff) * 25 + 30
            if off >= (self.lcd.height - 10):
                break

            selection = "  "
            if self.current == i:
                selection = "->"
            name, mac, rssi, timeout, device = self.results[i]
            age = int(time.time() - timeout)
            s1 = "{}: {}".format(i + 1, name)
            s2 = "{} [{}] {} {}".format(selection, mac, rssi, age)

            c1 = self.lcd.white
            if name == "S&B VOLCANO H":
                c1 = self.lcd.green
                if self.current == None:
                    self.current = i
            elif name == "STORZ&BICKEL":
                c1 = self.lcd.yellow
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
            if keys.once("enter") or keys.once("a"):
                if self.current < len(self.results):
                    return 2
            elif keys.once("up"):
                if self.current == None:
                    self.current = len(self.results) - 1
                else:
                    self.current -= 1
            elif keys.once("down"):
                if self.current == None:
                    self.current = 0
                else:
                    self.current += 1

            if self.current != None:
                while self.current < 0:
                    self.current += len(self.results)
                while self.current >= len(self.results):
                    self.current -= len(self.results)
                while self.current < self.menuOff:
                    self.menuOff -= 1
                while self.current >= (self.menuOff + int((self.lcd.height - 30 - 10) / 25)):
                    self.menuOff += 1

            # remove entries after timeout
            self.results = [x for x in self.results if (time.time() - x[3]) < 10.0]

            # filter out incompatible devices
            self.results = [x for x in self.results if (x[0] != None) and (("S&B" in x[0]) or ("STORZ&BICKEL" == x[0]))]

            if self.current != None:
                if self.current >= len(self.results):
                    self.current = len(self.results) - 1

            self.draw_list()

        return -1
