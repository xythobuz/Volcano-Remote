#!/usr/bin/env python

import uasyncio as asyncio
import bluetooth
import time

from scan import ble_scan

serviceUuidVolcano3 = bluetooth.UUID("10100000-5354-4f52-5a26-4249434b454c")
serviceUuidVolcano4 = bluetooth.UUID("10110000-5354-4f52-5a26-4249434b454c")

async def ble_conn(address):
    dev = await ble_scan(address)

    if dev:
        address = dev.addr_hex()
        print("Connecting to '{}'...".format(address))
        connection = await dev.connect()
        return connection

    return None

async def get_current_temp(device):
    service = await device.service(serviceUuidVolcano4)
    uuid = bluetooth.UUID("10110001-5354-4f52-5a26-4249434b454c")
    characteristic = await service.characteristic(uuid)
    val = await characteristic.read()
    num = int.from_bytes(val, "little")
    return num / 10.0

async def get_target_temp(device):
    service = await device.service(serviceUuidVolcano4)
    uuid = bluetooth.UUID("10110003-5354-4f52-5a26-4249434b454c")
    characteristic = await service.characteristic(uuid)
    val = await characteristic.read()
    num = int.from_bytes(val, "little")
    return num / 10.0

async def set_target_temp(device, temp):
    attempts = 3
    while attempts > 0:
        val = int(temp * 10.0)
        d = val.to_bytes(4, "little")
        service = await device.service(serviceUuidVolcano4)
        uuid = bluetooth.UUID("10110003-5354-4f52-5a26-4249434b454c")
        characteristic = await service.characteristic(uuid)
        await characteristic.write(d)

        attempts -= 1

        target = await get_target_temp(device)
        if abs(target - temp) < 0.5:
            return
    raise RuntimeError("Could not set target temperature")

async def get_unit_is_fahrenheit(device):
    service = await device.service(serviceUuidVolcano3)
    uuid = bluetooth.UUID("1010000d-5354-4f52-5a26-4249434b454c")
    characteristic = await service.characteristic(uuid)
    val = await characteristic.read()
    num = int.from_bytes(val, "little")
    return (num & 0x200) != 0

async def get_state(device):
    service = await device.service(serviceUuidVolcano3)
    uuid = bluetooth.UUID("1010000c-5354-4f52-5a26-4249434b454c")
    characteristic = await service.characteristic(uuid)
    val = await characteristic.read()
    num = int.from_bytes(val, "little")
    heater = (num & 0x0020) != 0
    pump = (num & 0x2000) != 0
    return (heater, pump)

async def set_state(device, state):
    heater, pump = state
    if heater:
        service = await device.service(serviceUuidVolcano4)
        uuid = bluetooth.UUID("1011000f-5354-4f52-5a26-4249434b454c")
        characteristic = await service.characteristic(uuid)
        await characteristic.write(int(0).to_bytes(1, "little"))
    else:
        service = await device.service(serviceUuidVolcano4)
        uuid = bluetooth.UUID("10110010-5354-4f52-5a26-4249434b454c")
        characteristic = await service.characteristic(uuid)
        await characteristic.write(int(0).to_bytes(1, "little"))
    if pump:
        service = await device.service(serviceUuidVolcano4)
        uuid = bluetooth.UUID("10110013-5354-4f52-5a26-4249434b454c")
        characteristic = await service.characteristic(uuid)
        await characteristic.write(int(0).to_bytes(1, "little"))
    else:
        service = await device.service(serviceUuidVolcano4)
        uuid = bluetooth.UUID("10110014-5354-4f52-5a26-4249434b454c")
        characteristic = await service.characteristic(uuid)
        await characteristic.write(int(0).to_bytes(1, "little"))

if __name__ == "__main__":
    async def test_poll(device):
        temp = await get_current_temp(device)
        print("Current Temperature: {}".format(temp))

        target = await get_target_temp(device)
        print("Target Temperature: {}".format(target))

        fahrenheit = await get_unit_is_fahrenheit(device)
        if fahrenheit:
            print("Unit is Fahrenheit")
        else:
            print("Unit is Celsius")

        heater, pump = await get_state(device)
        if heater:
            print("Heater is On")
        else:
            print("Heater is Off")
        if pump:
            print("Pump is On")
        else:
            print("Pump is Off")

    async def test(address):
        device = await ble_conn(address)
        if device == None:
            return

        async with device:
            print("Writing...")
            await set_target_temp(device, 190.0)

            print("Reading...")
            for i in range(0, 5):
                await test_poll(device)
                print()
                time.sleep(2.0)

    asyncio.run(test(None))
