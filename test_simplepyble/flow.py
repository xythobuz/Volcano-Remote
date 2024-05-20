#!/usr/bin/env python

import sys
import time
import os

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

def sleep(t):
    w = terminal_width
    if t < w:
        w = int(t)
    print_bar(0, 0, w, "s")
    for i in range(0, w):
        time.sleep(t / w)
        print_bar(i + 1, 0, w, "s")
    print()

def wait_for_temp(client, temp):
    print("Setting temperature {}".format(temp))
    set_target_temp(client, temp)

    print("Waiting for temperature to rise...")
    start = get_current_temp(client)
    curr = start
    print_bar(curr, start, temp, " degC")
    while curr < temp:
        time.sleep(1.0)
        curr = get_current_temp(client)
        print_bar(curr, start, temp, " degC")
    print()

    print("Reached temperature {}".format(temp))

def flow_step(client, temp, t_wait, t_pump):
    wait_for_temp(client, temp)

    print("Waiting {}s for heat to settle...".format(t_wait))
    sleep(t_wait)

    print("Pumping for {}s".format(t_pump))
    set_state(client, (True, True)) # turn on pump
    sleep(t_pump)
    set_state(client, (True, False)) # turn off pump

def flow(client):
    print("Turning on heater")
    set_state(client, (True, False))

    flow_step(client, 185.0, 10.0, 5.0)
    flow_step(client, 195.0, 5.0, 20.0)
    flow_step(client, 205.0, 5.0, 20.0)

    print("Notification by pumping three times...")
    for i in range(0, 3):
        time.sleep(1.0)
        set_state(client, (True, True)) # turn on pump
        time.sleep(1.0)
        set_state(client, (True, False)) # turn off pump

    print("Turning heater off")
    set_state(client, (False, False)) # turn off heater and pump

    print("Resetting temperature")
    set_target_temp(client, 190.0)

if __name__ == "__main__":
    def main(address, adapter):
        client = ble_conn(address, adapter)

        try:
            if get_unit_is_fahrenheit(client):
                raise RuntimeError("Imperial American scum is currently not supported :P")

            print("Starting Workflow")
            flow(client)
        except:
            print("\nTurning heater and pump off")
            set_state(client, (False, False)) # turn off heater and pump

            print("Disconnecting")
            client.disconnect()

            raise

        print("Disconnecting")
        client.disconnect()

    adapter = None
    mac = None
    if len(sys.argv) > 1:
        adapter = int(sys.argv[1])
    if len(sys.argv) > 2:
        mac = sys.argv[2]

    main(mac, adapter)
