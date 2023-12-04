/*
 * venty.c
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
#include "main.h"
#include "venty.h"

#define VENTY_READ_TIMEOUT_MS 500

enum venty_values {
    MASK_SET_TEMPERATURE = 1 << 1,
    MASK_SET_BOOST       = 1 << 2,
    MASK_SET_SUPERBOOST  = 1 << 3,
    MASK_HEATER          = 1 << 5,
    MASK_SETTINGS        = 1 << 7,
};

enum venty_settings {
    SETTING_UNIT                = 1 << 0,
    SETTING_SETPOINT_REACHED    = 1 << 1,
    SETTING_FACTORY_RESET       = 1 << 2,
    SETTING_ECOMODE_CHARGE      = 1 << 3,
    SETTING_BUTTON_CHANGED      = 1 << 4,
    SETTING_ECOMODE_VOLTAGE     = 1 << 5,
    SETTING_BOOST_VISUALIZATION = 1 << 6,
};

// "00000000-5354-4f52-5a26-4249434b454c"
static uint8_t uuid_service[16] = {
    0x00, 0x00, 0x00, 0x00, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};

// "00000001-5354-4f52-5a26-4249434b454c"
static uint8_t uuid_characteristic[16] = {
    0x00, 0x00, 0x00, 0x01, 0x53, 0x54, 0x4f, 0x52,
    0x5a, 0x26, 0x42, 0x49, 0x43, 0x4b, 0x45, 0x4c,
};

static int8_t venty_cmd(uint8_t *rx, size_t rx_len,
                        const uint8_t *tx, size_t tx_len) {
    int8_t r = ble_notification_enable(uuid_service, uuid_characteristic);
    if (r < 0) {
        debug("ble_notification_enable failed: %d", r);
        ble_notification_disable(uuid_service, uuid_characteristic);
        return r;
    }

    r = ble_write(uuid_service, uuid_characteristic,
                         tx, tx_len);
    if (r < 0) {
        debug("ble_write failed: %d", r);
        ble_notification_disable(uuid_service, uuid_characteristic);
        return r;
    }

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while (!ble_notification_ready()) {
        sleep_ms(1);
        main_loop_hw();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_time) >= VENTY_READ_TIMEOUT_MS) {
            debug("timeout waiting for notification");
            ble_notification_disable(uuid_service, uuid_characteristic);
            return -1;
        }
    }

    uint8_t uuid[16] = {0};
    uint16_t len = ble_notification_get(rx, rx_len, uuid);

    if (memcmp(uuid, uuid_characteristic, 16) != 0) {
        debug("got notification for unexpected uuid");
        len = 0;
    }

    ble_notification_disable(uuid_service, uuid_characteristic);
    return (len <= 0) ? -1 : 0;
}

int16_t venty_get_target_temp(void) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    if (r < 0) {
        return r;
    }

    int16_t *val = (int16_t *)(buff + 4);
    return *val;
}

int8_t venty_set_target_temp(uint16_t value) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;
    cmd[1] = MASK_SET_TEMPERATURE;
    uint16_t *val = (uint16_t *)(cmd + 4);
    *val = value;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    return r;
}

int8_t venty_set_heater_state(bool value) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;
    cmd[1] = MASK_HEATER;
    cmd[11] = value ? 1 : 0;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    return r;
}

int8_t venty_get_battery_state(void) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    if (r < 0) {
        return r;
    }

    int8_t *val = (int8_t *)(buff + 8);
    return *val;
}

int8_t venty_get_eco_current(void) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    if (r < 0) {
        return r;
    }

    int8_t *val = (int8_t *)(buff + 14);
    return (*val & SETTING_ECOMODE_CHARGE) ? 1 : 0;

}

int8_t venty_get_eco_voltage(void) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    if (r < 0) {
        return r;
    }

    int8_t *val = (int8_t *)(buff + 14);
    return (*val & SETTING_ECOMODE_VOLTAGE) ? 1 : 0;
}

int8_t venty_set_eco_current(bool value) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;
    cmd[1] = MASK_SETTINGS;
    cmd[14] = value ? SETTING_ECOMODE_CHARGE : 0;
    cmd[15] = SETTING_ECOMODE_CHARGE;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    return r;
}

int8_t venty_set_eco_voltage(bool value) {
    uint8_t cmd[20] = {0};
    uint8_t buff[20] = {0};

    cmd[0] = 1;
    cmd[1] = MASK_SETTINGS;
    cmd[14] = value ? SETTING_ECOMODE_VOLTAGE : 0;
    cmd[15] = SETTING_ECOMODE_VOLTAGE;

    int8_t r = venty_cmd(buff, sizeof(buff), cmd, sizeof(cmd));
    return r;
}
