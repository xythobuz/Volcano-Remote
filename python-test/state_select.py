#!/usr/bin/env python

import uasyncio as asyncio
from workflows import workflows

class StateSelect:
    def __init__(self, lcd):
        self.lcd = lcd

    def enter(self, val = None):
        self.client = val
        self.current = 0
        self.menuOff = 0

    def exit(self):
        return self.client, workflows[self.current]

    def draw_list(self):
        for i, wf in enumerate(workflows):
            if i < self.menuOff:
                continue

            off = (i - self.menuOff) * 25 + 30
            if off >= (self.lcd.height - 10):
                break

            s1 = "{}".format(wf["name"])
            s2 = "by: {}".format(wf["author"])

            c = self.lcd.white
            if self.current == i:
                c = self.lcd.red

            self.lcd.hline(0, off, self.lcd.width, self.lcd.blue)
            self.lcd.text(s1, 0, off + 2, c)
            self.lcd.text(s2, 0, off + 12, c)

    async def draw(self):
        self.lcd.text("Please select your Workflow", 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            return 5
        elif keys.once("up"):
            self.current -= 1
        elif keys.once("down"):
            self.current += 1
        elif keys.once("enter") or keys.once("a"):
            return 1

        while self.current < 0:
            self.current += len(workflows)
        while self.current >= len(workflows):
            self.current -= len(workflows)
        while self.current < self.menuOff:
            self.menuOff -= 1
        while self.current >= (self.menuOff + int((self.lcd.height - 30 - 10) / 25)):
            self.menuOff += 1

        self.draw_list()

        return -1
