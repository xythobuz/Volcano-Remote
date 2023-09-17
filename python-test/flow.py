#!/usr/bin/env python

import sys
import asyncio
from poll import (
    ble_conn,
    get_current_temp,
    get_target_temp, set_target_temp,
    get_unit_is_fahrenheit,
    get_state, set_state
)

async def wait_for_temp(client, temp):
    print("Setting temperature {}".format(temp))
    await set_target_temp(client, temp)

    print("Waiting for temperature to rise...")
    curr = await get_current_temp(client)
    while curr < temp:
        print("Currently at {}".format(curr))
        await asyncio.sleep(2.0)
        curr = await get_current_temp(client)

    print("Reached temperature {}".format(temp))

async def flow_step(client, temp, t_wait, t_pump):
    await wait_for_temp(client, temp)

    print("Waiting {}s for heat to settle...".format(t_wait))
    await asyncio.sleep(t_wait)

    print("Pumping for {}s".format(t_pump))
    await set_state(client, (True, True)) # turn on pump
    await asyncio.sleep(t_pump)
    await set_state(client, (True, False)) # turn off pump

async def flow(client):
    print("Turning on heater")
    await set_state(client, (True, False))

    await flow_step(client, 190.0, 25.0, 5.0)
    await flow_step(client, 205.0, 10.0, 20.0)
    await flow_step(client, 220.0, 10.0, 20.0)

    print("Notification by pumping four times...")
    for i in range(0, 4):
        await asyncio.sleep(1.0)
        await set_state(client, (True, True)) # turn on pump
        await asyncio.sleep(1.0)
        await set_state(client, (True, False)) # turn off pump

    print("Turning heater off")
    await set_state(client, (False, False)) # turn off heater and pump

async def main(address):
    device = await ble_conn(address)

    print("Connecting...")
    async with device as client:
        try:
            print("Starting Workflow")
            await flow(client)
        except asyncio.exceptions.CancelledError:
            print("Turning heater off")
            await set_state(client, (False, False)) # turn off heater and pump
        except KeyboardInterrupt:
            print("Turning heater off")
            await set_state(client, (False, False)) # turn off heater and pump

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Please pass MAC address of device")
        sys.exit(1)

    asyncio.run(main(sys.argv[1]))
