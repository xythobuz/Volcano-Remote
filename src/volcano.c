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
#include "ble.h"
#include "volcano.h"

#define UUID_SRVC_1       0x10
#define UUID_PRJSTAT1     0x0C
#define UUID_PRJSTAT2     0x0D
#define UUID_PRJSTAT3     0x0E

#define UUID_SRVC_2       0x11
#define UUID_WRITE_SRVC   0x00
#define UUID_CURRENT_TEMP 0x01
#define UUID_TARGET_TEMP  0x03
#define UUID_HEATER_ON    0x0F
#define UUID_HEATER_OFF   0x10
#define UUID_PUMP_ON      0x13
#define UUID_PUMP_OFF     0x14

#define MASK_PRJSTAT1_HEIZUNG_ENA        0x0020
#define MASK_PRJSTAT1_AUTOBLESHUTDOWN    0x0200
#define MASK_PRJSTAT1_PUMPE_FET_ENABLE   0x2000

#define MASK_PRJSTAT2_FAHRENHEIT_ENA     0x0200
#define MASK_PRJSTAT2_DISPLAY_ON_COOLING 0x1000

#define MASK_PRJSTAT3_VIBRATION          0x0400

// "10xx00xx-5354-4f52-5a26-4249434b454c"
static uint8_t uuid_base[16] = {
    0x10, 0xFF, 0x00, 0xFF, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};
static uint8_t uuid_base2[16] = {
    0x10, 0xFF, 0x00, 0xFF, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};

int8_t volcano_discover_characteristics(void) {
    uuid_base[1] = UUID_SRVC_2;
    uuid_base2[1] = UUID_SRVC_2;

    uuid_base[3] = UUID_WRITE_SRVC;
    int8_t r;

    uuid_base2[3] = UUID_TARGET_TEMP;
    r = ble_discover(uuid_base, uuid_base2);
    if (r < 0) {
        return r;
    }

    uuid_base2[3] = UUID_HEATER_ON;
    r = ble_discover(uuid_base, uuid_base2);
    if (r < 0) {
        return r;
    }

    uuid_base2[3] = UUID_HEATER_OFF;
    r = ble_discover(uuid_base, uuid_base2);
    if (r < 0) {
        return r;
    }

    uuid_base2[3] = UUID_PUMP_ON;
    r = ble_discover(uuid_base, uuid_base2);
    if (r < 0) {
        return r;
    }

    uuid_base2[3] = UUID_PUMP_OFF;
    r = ble_discover(uuid_base, uuid_base2);
    if (r < 0) {
        return r;
    }

    return 0;
}

int16_t volcano_get_current_temp(void) {
    uuid_base[1] = UUID_SRVC_2;
    uuid_base[3] = UUID_CURRENT_TEMP;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return *v;
}

int16_t volcano_get_target_temp(void) {
    uuid_base[3] = UUID_TARGET_TEMP;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return *v;
}

int8_t volcano_set_target_temp(uint16_t value) {
    uuid_base[1] = UUID_SRVC_2;
    uuid_base2[1] = UUID_SRVC_2;
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_TARGET_TEMP;

    uint8_t buff[4];
    uint32_t *v = (uint32_t *)buff;
    *v = value;

    int8_t r = ble_write(uuid_base, uuid_base2, buff, sizeof(buff));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t volcano_set_heater_state(bool value) {
    uuid_base[1] = UUID_SRVC_2;
    uuid_base2[1] = UUID_SRVC_2;
    uuid_base[3] = UUID_WRITE_SRVC;

    if (value) {
        uuid_base2[3] = UUID_HEATER_ON;
    } else {
        uuid_base2[3] = UUID_HEATER_OFF;
    }

    uint8_t d = 0;
    int8_t r = ble_write(uuid_base, uuid_base2, &d, sizeof(d));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t volcano_set_pump_state(bool value) {
    uuid_base[1] = UUID_SRVC_2;
    uuid_base2[1] = UUID_SRVC_2;
    uuid_base[3] = UUID_WRITE_SRVC;

    if (value) {
        uuid_base2[3] = UUID_PUMP_ON;
    } else {
        uuid_base2[3] = UUID_PUMP_OFF;
    }

    uint8_t d = 0;
    int8_t r = ble_write(uuid_base, uuid_base2, &d, sizeof(d));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

enum unit volcano_get_unit(void) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_PRJSTAT2;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return UNIT_INVALID;
    }

    uint32_t *v = (uint32_t *)buff;
    return (*v & MASK_PRJSTAT2_FAHRENHEIT_ENA) ? UNIT_F : UNIT_C;
}

enum volcano_state volcano_get_state(void) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_PRJSTAT1;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return VOLCANO_STATE_INVALID;
    }

    uint32_t *v = (uint32_t *)buff;
    uint32_t heater = (*v & MASK_PRJSTAT1_HEIZUNG_ENA);
    uint32_t pump = (*v & MASK_PRJSTAT1_PUMPE_FET_ENABLE);
    return (heater ? VOLCANO_STATE_HEATER : 0) | (pump ? VOLCANO_STATE_PUMP : 0);
}

int8_t volcano_set_unit(enum unit unit) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base2[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_PRJSTAT2;

    uint32_t v = MASK_PRJSTAT2_FAHRENHEIT_ENA;
    if (unit == UNIT_F) {
        v |= 0x10000;
    }

    int8_t r = ble_write(uuid_base, uuid_base2, (uint8_t *)&v, sizeof(v));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t volcano_set_vibration(bool value) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base2[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_PRJSTAT3;

    uint32_t v = MASK_PRJSTAT3_VIBRATION;
    if (!value) {
        v |= 0x10000;
    }

    int8_t r = ble_write(uuid_base, uuid_base2, (uint8_t *)&v, sizeof(v));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;
}

int8_t volcano_get_vibration(void) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_PRJSTAT3;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return (*v & MASK_PRJSTAT3_VIBRATION) ? 0 : 1;
}

int8_t volcano_set_display_cooling(bool value) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base2[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_WRITE_SRVC;
    uuid_base2[3] = UUID_PRJSTAT2;

    uint32_t v = MASK_PRJSTAT2_DISPLAY_ON_COOLING;
    if (!value) {
        v |= 0x10000;
    }

    int8_t r = ble_write(uuid_base, uuid_base2, (uint8_t *)&v, sizeof(v));
    if (r != 0) {
        debug("ble_write unexpected value %d", r);
    }
    return r;

}

int8_t volcano_get_display_cooling(void) {
    uuid_base[1] = UUID_SRVC_1;
    uuid_base[3] = UUID_PRJSTAT2;

    uint8_t buff[4];
    int32_t r = ble_read(uuid_base, buff, sizeof(buff));
    if (r != sizeof(buff)) {
        debug("ble_read unexpected value %ld", r);
        return -1;
    }

    uint32_t *v = (uint32_t *)buff;
    return (*v & MASK_PRJSTAT2_DISPLAY_ON_COOLING) ? 0 : 1;
}
