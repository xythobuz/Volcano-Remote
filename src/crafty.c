/*
 * crafty.c
 *
 * Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "log.h"
#include "ble.h"
#include "crafty.h"

// serviceUuidCrafty1      "00000001-4c45-4b43-4942-265a524f5453"
// characteristicWriteTemp "00000021-4c45-4b43-4942-265a524f5453"
// charactersiticCurrTemp  "00000011-4c45-4b43-4942-265a524f5453"
// characteristicPower     "00000041-4c45-4b43-4942-265a524f5453"
// characteristicHeaterOn  "00000081-4c45-4b43-4942-265a524f5453"
// characteristicHeaterOff "00000091-4c45-4b43-4942-265a524f5453"

// "000000xx-4c45-4b43-4942-265a524f5453"
static uint8_t uuid_base[16] = {
    0x00, 0x00, 0x00, 0xFF, 0x4c, 0x45, 0x4b, 0x43,
    0x49, 0x42, 0x26, 0x5a, 0x52, 0x4f, 0x54, 0x53,
};
static uint8_t uuid_base2[16] = {
    0x00, 0x00, 0x00, 0xFF, 0x4c, 0x45, 0x4b, 0x43,
    0x49, 0x42, 0x26, 0x5a, 0x52, 0x4f, 0x54, 0x53,
};

// Crafty UUIDs are always the same, except for the 4th byte
#define UUID_WRITE_SRVC   0x01
#define UUID_CURRENT_TEMP 0x11
#define UUID_TARGET_TEMP  0x21
#define UUID_BATTERY      0x41
#define UUID_HEATER_ON    0x81
#define UUID_HEATER_OFF   0x91

int16_t crafty_get_current_temp(void) {
    uuid_base[3] = UUID_CURRENT_TEMP;

    uint8_t buff[2];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint16_t *v = (uint16_t *)buff;
    return *v;
}

int16_t crafty_get_target_temp(void) {
    uuid_base[3] = UUID_TARGET_TEMP;

    uint8_t buff[2];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint16_t *v = (uint16_t *)buff;
    return *v;
}

int8_t crafty_set_target_temp(uint16_t value) {
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_TARGET_TEMP;

    uint8_t buff[2];
    uint16_t *v = (uint16_t *)buff;
    *v = value;

    int8_t r = ble_write(uuid_base, uuid_base2, buff, sizeof(buff));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t crafty_set_heater_state(bool value) {
    uuid_base[3] = UUID_WRITE_SRVC;

    if (value) {
        uuid_base2[3] = UUID_HEATER_ON;
    } else {
        uuid_base2[3] = UUID_HEATER_OFF;
    }

    uint16_t d = 0;
    int8_t r = ble_write(uuid_base, uuid_base2, (uint8_t *)&d, sizeof(d));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t crafty_get_battery_state(void) {
    uuid_base[3] = UUID_BATTERY;

    uint8_t buff[2];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint16_t *v = (uint16_t *)buff;
    return *v;
}
