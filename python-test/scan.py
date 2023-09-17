#!/usr/bin/env python

import asyncio
from bleak import BleakScanner

async def scan():
    print("scanning for 5 seconds, please wait...")
    devices = await BleakScanner.discover(return_adv=True)
    for d, a in devices.values():
        print()
        print(d)
        print("-" * len(str(d)))
        print(a)

if __name__ == "__main__":
    asyncio.run(scan())
