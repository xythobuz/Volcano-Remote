#!/usr/bin/env python

import sys
import os
import asyncio
from poll import (
    ble_conn,
    get_current_temp,
    get_target_temp, set_target_temp,
    get_unit_is_fahrenheit,
    get_state, set_state
)

terminal_width = os.get_terminal_size().columns - 15

def print_bar(value, start, end, unit):
    width = terminal_width
    s = "\r"
    s += "#" * int((value - start) / (end - start) * width)
    s += "-" * (width - int((value - start) / (end - start) * width))
    s += " {}{}".format(value, unit)
    print(s, end="", flush=True)

async def sleep(t):
    w = terminal_width
    if t < w:
        w = int(t)
    print_bar(0, 0, w, "s")
    for i in range(0, w):
        await asyncio.sleep(t / w)
        print_bar(i + 1, 0, w, "s")
    print()

async def wait_for_temp(client, temp):
    print("Setting temperature {}".format(temp))
    await set_target_temp(client, temp)

    print("Waiting for temperature to rise...")
    start = await get_current_temp(client)
    curr = start
    print_bar(curr, start, temp, " degC")
    while curr < temp:
        await asyncio.sleep(1.0)
        curr = await get_current_temp(client)
        print_bar(curr, start, temp, " degC")
    print()

    print("Reached temperature {}".format(temp))

async def flow_step(client, temp, t_wait, t_pump):
    await wait_for_temp(client, temp)

    print("Waiting {}s for heat to settle...".format(t_wait))
    await sleep(t_wait)

    print("Pumping for {}s".format(t_pump))
    await set_state(client, (True, True)) # turn on pump
    await sleep(t_pump)
    await set_state(client, (True, False)) # turn off pump

async def flow(client):
    print("Turning on heater")
    await set_state(client, (True, False))

    await flow_step(client, 185.0, 10.0, 7.0)
    await flow_step(client, 195.0, 5.0, 23.0)
    await flow_step(client, 205.0, 5.0, 23.0)

    print("Notification by pumping three times...")
    for i in range(0, 3):
        await asyncio.sleep(1.0)
        await set_state(client, (True, True)) # turn on pump
        await asyncio.sleep(1.0)
        await set_state(client, (True, False)) # turn off pump

    print("Turning heater off")
    await set_state(client, (False, False)) # turn off heater and pump

    print("Resetting temperature")
    await set_target_temp(client, 190.0)

async def main(address):
    device = await ble_conn(address)

    print("Connecting...")
    async with device as client:
        try:
            if await get_unit_is_fahrenheit(client):
                print("Imperial American scum is currently not supported :P")
                sys.exit(42)

            print("Starting Workflow")
            await flow(client)
        except:
            print("\nTurning heater and pump off")
            await set_state(client, (False, False)) # turn off heater and pump
            raise

        print("Disconnecting...")

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Please pass MAC address of device")
        sys.exit(1)

    asyncio.run(main(sys.argv[1]))
