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

terminal_width = 100 #os.get_terminal_size().columns - 15

def print_bar(value, start, end, unit):
    width = terminal_width
    s = "\r"
    s += "#" * int((value - start) / (end - start) * width)
    s += "-" * (width - int((value - start) / (end - start) * width))
    s += " {}{}".format(value, unit)
    print(s, end="")

def sleep(t):
    print_bar(0, 0, t, "s")
    for i in range(0, t):
        time.sleep(1.0)
        print_bar(i + 1, 0, t, "s")
    print()

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

    await flow_step(client, 190.0, 15.0, 5.0 - 4)
    await flow_step(client, 205.0, 10.0, 20.0 - 4)
    await flow_step(client, 220.0, 10.0, 20.0 - 4)

    print("Notification by pumping three times...")
    for i in range(0, 3):
        #time.sleep(1.0 / 3)
        await set_state(client, (True, True)) # turn on pump
        #time.sleep(1.0 / 3)
        await set_state(client, (True, False)) # turn off pump

    print("Turning heater off")
    await set_state(client, (False, False)) # turn off heater and pump

    print("Setting temperature back to 190")
    await set_target_temp(client, 190.0)

if __name__ == "__main__":
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

    import machine
    led_onboard = machine.Pin("LED", machine.Pin.OUT)

    for i in range(0, 3):
        led_onboard.on()
        time.sleep(0.2)
        led_onboard.off()
        time.sleep(0.2)

    print("ready")
    while True:
        if rp2.bootsel_button() == 1:
            led_onboard.on()
            print("run")

            try:
                asyncio.run(main(None))
                print("done")
            except Exception as e:
                print(e)

            led_onboard.off()
            machine.reset()

        time.sleep(0.2)
