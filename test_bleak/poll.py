#!/usr/bin/env python

import sys
import asyncio
from bleak import BleakClient, BleakScanner
from bleak.exc import BleakDBusError
import aiohttp
import asyncio

influx_host = 'http://INFLUX_DB_IP_HERE:8086'
influx_path = '/write?db=INFLUX_DB_NAME_HERE'

cache = {}

async def influx(name, value):
    if not name in cache:
        cache[name] = value
    elif cache[name] == value:
        return

    data = "volcano,device=bleak " + name + "=" + str(float(value))
    async with aiohttp.ClientSession(influx_host) as session:
        await session.post(influx_path, data=data)

async def ble_conn(address):
    attempts = 10
    print("Opening device..", end = "", flush=True)
    while attempts > 0:
        try:
            if (attempts % 10) == 0:
                print(".", end = "", flush=True)

            device = await BleakScanner.find_device_by_address(address)
            client = BleakClient(device)
            print("", flush=True)
            return client
        except BleakDBusError as e:
            attempts -= 1
            if attempts == 0:
                print("", flush=True)
                print(e)
            else:
                await asyncio.sleep(0.1)

    print("Could not connect to device")
    return None

async def get_current_temp(client):
    val = await client.read_gatt_char("10110001-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    await influx("current", num)
    return num / 10.0

async def get_target_temp(client):
    val = await client.read_gatt_char("10110003-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    return num / 10.0

async def set_target_temp(client, temp):
    val = int(temp * 10.0)
    await influx("target", val)
    d = val.to_bytes(4, byteorder="little")
    await client.write_gatt_char("10110003-5354-4f52-5a26-4249434b454c", d)

async def get_unit_is_fahrenheit(client):
    val = await client.read_gatt_char("1010000d-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    return (num & 0x200) != 0

async def get_state(client):
    val = await client.read_gatt_char("1010000c-5354-4f52-5a26-4249434b454c")
    num = int.from_bytes(val, byteorder="little")
    heater = (num & 0x0020) != 0
    pump = (num & 0x2000) != 0
    return (heater, pump)

async def set_state(client, state):
    heater, pump = state
    await influx("heater", heater)
    await influx("pump", pump)
    if heater:
        await client.write_gatt_char("1011000f-5354-4f52-5a26-4249434b454c", 0)
    else:
        await client.write_gatt_char("10110010-5354-4f52-5a26-4249434b454c", 0)
    if pump:
        await client.write_gatt_char("10110013-5354-4f52-5a26-4249434b454c", 0)
    else:
        await client.write_gatt_char("10110014-5354-4f52-5a26-4249434b454c", 0)

async def test_poll(client):
    temp = await get_current_temp(client)
    print("Current Temperature: {}".format(temp))

    target = await get_target_temp(client)
    print("Target Temperature: {}".format(target))

    fahrenheit = await get_unit_is_fahrenheit(client)
    if fahrenheit:
        print("Unit is Fahrenheit")
    else:
        print("Unit is Celsius")

    heater, pump = await get_state(client)
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

    print("Connecting...")
    async with device as client:
        print("Writing...")
        await set_target_temp(client, 190.0)

        print("Reading...")
        for i in range(0, 5):
            await test_poll(client)
            print()
            await asyncio.sleep(2.0)

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Please pass MAC address of device")
        sys.exit(1)

    asyncio.run(test(sys.argv[1]))
