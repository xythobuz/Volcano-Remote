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

# https://github.com/micropython/micropython-lib/blob/master/micropython/bluetooth/aioble/examples/temp_client.py

import uasyncio as asyncio
import aioble
import bluetooth
import sys

async def ble_scan(addr = None, name = "S&B VOLCANO H", timeout = 0.5):
    scanner = aioble.scan(int(timeout * 1000.0), interval_us=30000, window_us=30000, active=True)
    async with scanner as s:
        results = []
        async for d in s:
            if addr != None:
                if addr == d.device.addr_hex():
                    return d
            elif name != None:
                if d.name() == name:
                    return d
            else:
                results.append(d)
        return results

    return None
