# https://github.com/micropython/micropython-lib/blob/master/micropython/bluetooth/aioble/examples/temp_client.py

import uasyncio as asyncio
import aioble
import bluetooth
import sys

async def ble_scan(addr = None, timeout = 0.1):
    print("Scanning for '{}' for {}s...".format(addr, timeout))
    scanner = aioble.scan(int(timeout * 1000.0), interval_us=30000, window_us=30000, active=True)
    async with scanner as s:
        async for d in s:
            print("Scan: '{}' [{}]".format(d.name(), d.device.addr_hex()))
            if addr != None:
                if addr == d.device.addr_hex():
                    return d.device
            else:
                if d.name() == "S&B VOLCANO H":
                    return d.device

    print("No device found")
    return None

if __name__ == "__main__":
    dev = asyncio.run(ble_scan())
    if dev != None:
        print("{}".format(dev.addr_hex()))
