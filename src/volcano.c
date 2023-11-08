/*
 * volcano.c
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
#include "util.h"
#include "ble.h"
#include "volcano.h"

// Volcano UUIDs are always the same, except for the 4th byte
#define UUID_WRITE_SRVC   0x00
#define UUID_CURRENT_TEMP 0x01
#define UUID_TARGET_TEMP  0x03
#define UUID_HEATER_ON    0x0F
#define UUID_HEATER_OFF   0x10
#define UUID_PUMP_ON      0x13
#define UUID_PUMP_OFF     0x14

// "101100xx-5354-4f52-5a26-4249434b454c"
static uint8_t uuid_base[16] = {
    0x10, 0x11, 0x00, 0xFF, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};
static uint8_t uuid_base2[16] = {
    0x10, 0x11, 0x00, 0xFF, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};

int16_t volcano_get_current_temp(void) {
    uuid_base[3] = UUID_CURRENT_TEMP;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, 4);
    if (r != 4) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return *v;
}

int16_t volcano_get_target_temp(void) {
    uuid_base[3] = UUID_TARGET_TEMP;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, 4);
    if (r != 4) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return *v;
}

int8_t volcano_set_target_temp(uint16_t value) {
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_TARGET_TEMP;

    uint8_t buff[4];
    uint32_t *v = (uint32_t *)buff;
    *v = value;

    int8_t r = ble_write(uuid_base, uuid_base2, buff, 4);
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}
