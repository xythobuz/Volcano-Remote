#!/usr/bin/env python

import time
from state_wait_temp import draw_graph

class StateWaitTime:
    def __init__(self, lcd):
        self.lcd = lcd

    def enter(self, val = None):
        self.value = val

        device, workflow, index = self.value
        self.start = time.time()
        self.end = self.start + workflow["steps"][index][1]

    def exit(self):
        return (self.value[0], self.value[1], self.value[2])

    async def draw(self):
        self.lcd.fill(self.lcd.black)

        device, workflow, index = self.value
        self.lcd.text("Volcano Remote Control App", 0, 0, self.lcd.green)
        self.lcd.text("Running Workflow - Wait {}".format(workflow["steps"][index][1]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            print("user abort")
            return 4 # heat off

        now = time.time()
        draw_graph(self.lcd, 0.0, now - self.start, self.end - self.start)
        if now >= self.end:
            print("switch, {} >= {}".format(now, self.end))
            return 8 # pump

        self.lcd.show()
        return -1 # stay in this state
