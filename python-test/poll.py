#!/usr/bin/env python

import simplepyble
from scan import ble_scan
import time

serviceUuidVolcano3 = "10100000-5354-4f52-5a26-4249434b454c"
serviceUuidVolcano4 = "10110000-5354-4f52-5a26-4249434b454c"

def ble_conn(address):
    dev = ble_scan(address)

    if dev != None:
        print("Connecting to {}...".format(address))
        dev.connect()

    return dev

def get_current_temp(device):
    val = device.read(serviceUuidVolcano4, "10110001-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    return num / 10.0

def get_target_temp(device):
    val = device.read(serviceUuidVolcano4, "10110003-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    return num / 10.0

def set_target_temp(device, temp):
    val = int(temp * 10.0)
    d = val.to_bytes(4, byteorder="little")
    device.write_request(serviceUuidVolcano4, "10110003-5354-4f52-5a26-4249434b454c", d)

def get_unit_is_fahrenheit(device):
    val = device.read(serviceUuidVolcano3, "1010000d-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    return (num & 0x200) != 0

def get_state(device):
    val = device.read(serviceUuidVolcano3, "1010000c-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    heater = (num & 0x0020) != 0
    pump = (num & 0x2000) != 0
    return (heater, pump)

def set_state(device, state):
    heater, pump = state
    if heater:
        device.write_request(serviceUuidVolcano4, "1011000f-5354-4f52-5a26-4249434b454c", 0)
    else:
        device.write_request(serviceUuidVolcano4, "10110010-5354-4f52-5a26-4249434b454c", 0)
    if pump:
        device.write_request(serviceUuidVolcano4, "10110013-5354-4f52-5a26-4249434b454c", 0)
    else:
        device.write_request(serviceUuidVolcano4, "10110014-5354-4f52-5a26-4249434b454c", 0)

if __name__ == "__main__":
    def test_poll(device):
        temp = get_current_temp(device)
        print("Current Temperature: {}".format(temp))

        target = get_target_temp(device)
        print("Target Temperature: {}".format(target))

        fahrenheit = get_unit_is_fahrenheit(device)
        if fahrenheit:
            print("Unit is Fahrenheit")
        else:
            print("Unit is Celsius")

        heater, pump = get_state(device)
        if heater:
            print("Heater is On")
        else:
            print("Heater is Off")
        if pump:
            print("Pump is On")
        else:
            print("Pump is Off")

    def test(address):
        device = ble_conn(address)
        if device == None:
            return

        print("Writing...")
        set_target_temp(device, 190.0)

        print("Reading...")
        for i in range(0, 5):
            test_poll(device)
            print()
            time.sleep(2.0)

    import sys

    arg = None
    if len(sys.argv) > 1:
        arg = sys.argv[1]

    test(arg)
