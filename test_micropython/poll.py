#!/usr/bin/env python3

# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

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

    val = 0

    service3 = await device.service(serviceUuidVolcano3, 100)
    if cb != None:
        val += 1
        await cb(val)

    uuidc = bluetooth.UUID("1010000c-5354-4f52-5a26-4249434b454c")
    uuidd = bluetooth.UUID("1010000d-5354-4f52-5a26-4249434b454c")
    async for c in service3.characteristics(None, 100):
        if c.uuid == uuidc:
            characteristicc = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuidd:
            characteristicd = c
            if cb != None:
                val += 1
                await cb(val)

    # -------------------------------------------------------------------------

    service4 = await device.service(serviceUuidVolcano4, 100)
    if cb != None:
        val += 1
        await cb(val)

    uuid1 = bluetooth.UUID("10110001-5354-4f52-5a26-4249434b454c")
    uuid3 = bluetooth.UUID("10110003-5354-4f52-5a26-4249434b454c")
    uuidf = bluetooth.UUID("1011000f-5354-4f52-5a26-4249434b454c")
    uuid10 = bluetooth.UUID("10110010-5354-4f52-5a26-4249434b454c")
    uuid13 = bluetooth.UUID("10110013-5354-4f52-5a26-4249434b454c")
    uuid14 = bluetooth.UUID("10110014-5354-4f52-5a26-4249434b454c")
    async for c in service4.characteristics(None, 100):
        if c.uuid == uuid1:
            characteristic1 = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuid3:
            characteristic3 = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuidf:
            characteristicf = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuid10:
            characteristic10 = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuid13:
            characteristic13 = c
            if cb != None:
                val += 1
                await cb(val)

        if c.uuid == uuid14:
            characteristic14 = c
            if cb != None:
                val += 1
                await cb(val)

    return (val >= 10)

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

    attempts = 3
    while attempts > 0:
        attempts -= 1
        try:
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
