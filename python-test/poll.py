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

async def cache_services_characteristics(device, cb = None):
    global service3, service4
    global characteristic1, characteristic3
    global characteristicd, characteristicc
    global characteristicf, characteristic10
    global characteristic13, characteristic14

    service3 = await device.service(serviceUuidVolcano3)
    if cb != None:
        await cb(0.1)

    service4 = await device.service(serviceUuidVolcano4)
    if cb != None:
        await cb(0.2)

    uuid1 = bluetooth.UUID("10110001-5354-4f52-5a26-4249434b454c")
    characteristic1 = await service4.characteristic(uuid1)
    if cb != None:
        await cb(0.3)

    uuid3 = bluetooth.UUID("10110003-5354-4f52-5a26-4249434b454c")
    characteristic3 = await service4.characteristic(uuid3)
    if cb != None:
        await cb(0.4)

    uuidd = bluetooth.UUID("1010000d-5354-4f52-5a26-4249434b454c")
    characteristicd = await service3.characteristic(uuidd)
    if cb != None:
        await cb(0.5)

    uuidc = bluetooth.UUID("1010000c-5354-4f52-5a26-4249434b454c")
    characteristicc = await service3.characteristic(uuidc)
    if cb != None:
        await cb(0.6)

    uuidf = bluetooth.UUID("1011000f-5354-4f52-5a26-4249434b454c")
    characteristicf = await service4.characteristic(uuidf)
    if cb != None:
        await cb(0.7)

    uuid10 = bluetooth.UUID("10110010-5354-4f52-5a26-4249434b454c")
    characteristic10 = await service4.characteristic(uuid10)
    if cb != None:
        await cb(0.8)

    uuid13 = bluetooth.UUID("10110013-5354-4f52-5a26-4249434b454c")
    characteristic13 = await service4.characteristic(uuid13)
    if cb != None:
        await cb(0.9)

    uuid14 = bluetooth.UUID("10110014-5354-4f52-5a26-4249434b454c")
    characteristic14 = await service4.characteristic(uuid14)
    if cb != None:
        await cb(1.0)

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
    attempts = 3
    while attempts > 0:
        attempts -= 1
        try:
            heater, pump = state
            if heater == True:
                await characteristicf.write(int(0).to_bytes(1, "little"))
            elif heater == False:
                await characteristic10.write(int(0).to_bytes(1, "little"))

            if pump == True:
                await characteristic13.write(int(0).to_bytes(1, "little"))
            elif pump == False:
                await characteristic14.write(int(0).to_bytes(1, "little"))
        except:
            time.sleep(0.05)
            continue

        break
