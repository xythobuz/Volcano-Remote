#!/usr/bin/env python3

# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

import time
from state_wait_temp import draw_graph

class StateWaitTime:
    def __init__(self, lcd):
        self.lcd = lcd

    def enter(self, val = None):
        self.value = val

        device, workflow, index = self.value
        self.start = time.time()
        self.end = self.start + int(workflow["steps"][index][1])

    def exit(self):
        return (self.value[0], self.value[1], self.value[2])

    async def draw(self):
        device, workflow, index = self.value
        self.lcd.text("Running Workflow - Wait {}".format(workflow["steps"][index][1]), 0, 10, self.lcd.red)

        keys = self.lcd.buttons()

        if keys.once("y"):
            return 4

        now = time.time()
        draw_graph(self.lcd, 0.0, now - self.start, self.end - self.start)

        if now >= self.end:
            return 8

        return -1
