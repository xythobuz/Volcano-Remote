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
