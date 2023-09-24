#!/usr/bin/env python

import uasyncio as asyncio
import sys
import time

from poll import (
    ble_conn,
    get_current_temp,
    get_target_temp, set_target_temp,
    get_unit_is_fahrenheit,
    get_state, set_state
)

def print_bar(value, start, end, unit):
    print("{}{} -> {}{} -> {}{}".format(start, unit, value, unit, end, unit))

def sleep(t):
    print_bar(0, 0, t, "s")
    for i in range(0, t):
        time.sleep(1.0)
        print_bar(i + 1, 0, t, "s")

async def wait_for_temp(client, temp):
    print("Setting temperature {}".format(temp))
    await set_target_temp(client, temp)

    print("Waiting for temperature to rise...")
    start = await get_current_temp(client)
    curr = start
    print_bar(curr, start, temp, " degC")
    while curr < temp:
        time.sleep(1.0)
        curr = await get_current_temp(client)
        print_bar(curr, start, temp, " degC")
    print()

    print("Reached temperature {}".format(temp))

async def flow_step(client, temp, t_wait, t_pump):
    await wait_for_temp(client, temp)

    print("Waiting {}s for heat to settle...".format(t_wait))
    sleep(t_wait)

    print("Pumping for {}s".format(t_pump))
    await set_state(client, (True, True)) # turn on pump
    sleep(t_pump)
    await set_state(client, (True, False)) # turn off pump

async def flow(client):
    print("Turning on heater")
    await set_state(client, (True, False))

    await flow_step(client, 190.0, 20.0, 5.0 - 4.9)
    await flow_step(client, 205.0, 10.0, 20.0 - 7)
    await flow_step(client, 220.0, 10.0, 20.0 - 7)

    #print("Notification by pumping three times...")
    #for i in range(0, 3):
    #    time.sleep(1.0 / 3)
    #    await set_state(client, (True, True)) # turn on pump
    #    time.sleep(1.0 / 3)
    #    await set_state(client, (True, False)) # turn off pump

    print("Turning heater off")
    await set_state(client, (False, False)) # turn off heater and pump

    print("Setting temperature back to 190")
    await set_target_temp(client, 190.0)

if True:#__name__ == "__main__":
    async def main(address):
        client = await ble_conn(address)

        try:
            if await get_unit_is_fahrenheit(client):
                raise RuntimeError("Imperial American scum is currently not supported :P")

            print("Starting Workflow")
            await flow(client)
        except:
            print("\nTurning heater off")
            await set_state(client, (False, False)) # turn off heater and pump
            raise

    asyncio.run(main(None))
