#!/usr/bin/env python

import uasyncio as asyncio
import bluetooth
import time

from scan import ble_scan

serviceUuidVolcano3 = bluetooth.UUID("10100000-5354-4f52-5a26-4249434b454c")
serviceUuidVolcano4 = bluetooth.UUID("10110000-5354-4f52-5a26-4249434b454c")

service3 = None
service4 = None
characteristic1 = None
characteristic3 = None
characteristicd = None
characteristicc = None
characteristicf = None
characteristic10 = None
characteristic13 = None
characteristic14 = None

async def ble_conn(address):
    dev = await ble_scan(address)

    if dev:
        address = dev.device.addr_hex()
        print("Connecting to '{}'...".format(address))
        connection = await dev.device.connect()
        await cache_services_characteristics(connection)
        return connection

    return None

async def cache_services_characteristics(device):
    global service3, service4
    global characteristic1, characteristic3
    global characteristicd, characteristicc
    global characteristicf, characteristic10
    global characteristic13, characteristic14

    service3 = await device.service(serviceUuidVolcano3)
    service4 = await device.service(serviceUuidVolcano4)

    uuid1 = bluetooth.UUID("10110001-5354-4f52-5a26-4249434b454c")
    characteristic1 = await service4.characteristic(uuid1)

    uuid3 = bluetooth.UUID("10110003-5354-4f52-5a26-4249434b454c")
    characteristic3 = await service4.characteristic(uuid3)

    uuidd = bluetooth.UUID("1010000d-5354-4f52-5a26-4249434b454c")
    characteristicd = await service3.characteristic(uuidd)

    uuidc = bluetooth.UUID("1010000c-5354-4f52-5a26-4249434b454c")
    characteristicc = await service3.characteristic(uuidc)

    uuidf = bluetooth.UUID("1011000f-5354-4f52-5a26-4249434b454c")
    characteristicf = await service4.characteristic(uuidf)

    uuid10 = bluetooth.UUID("10110010-5354-4f52-5a26-4249434b454c")
    characteristic10 = await service4.characteristic(uuid10)

    uuid13 = bluetooth.UUID("10110013-5354-4f52-5a26-4249434b454c")
    characteristic13 = await service4.characteristic(uuid13)

    uuid14 = bluetooth.UUID("10110014-5354-4f52-5a26-4249434b454c")
    characteristic14 = await service4.characteristic(uuid14)

async def get_current_temp(device):
    val = await characteristic1.read()
    num = int.from_bytes(val, "little")
    return num / 10.0

async def get_target_temp(device):
    val = await characteristic3.read()
    num = int.from_bytes(val, "little")
    return num / 10.0

async def set_target_temp(device, temp):
    attempts = 3
    while attempts > 0:
        val = int(temp * 10.0)
        d = val.to_bytes(4, "little")
        await characteristic3.write(d)

        attempts -= 1

        target = await get_target_temp(device)
        if abs(target - temp) < 0.5:
            return
    raise RuntimeError("Could not set target temperature")

async def get_unit_is_fahrenheit(device):
    val = await characteristicd.read()
    num = int.from_bytes(val, "little")
    return (num & 0x200) != 0

async def get_state(device):
    val = await characteristicc.read()
    num = int.from_bytes(val, "little")
    heater = (num & 0x0020) != 0
    pump = (num & 0x2000) != 0
    return (heater, pump)

async def set_state(device, state):
    heater, pump = state
    if heater == True:
        await characteristicf.write(int(0).to_bytes(1, "little"))
    elif heater == False:
        await characteristic10.write(int(0).to_bytes(1, "little"))

    if pump == True:
        await characteristic13.write(int(0).to_bytes(1, "little"))
    elif pump == False:
        await characteristic14.write(int(0).to_bytes(1, "little"))

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
