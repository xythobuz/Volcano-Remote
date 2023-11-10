/*
 * models.c
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

#include <stddef.h>
#include <string.h>

#include "config.h"
#include "log.h"
#include "util.h"
#include "models.h"

enum known_devices models_filter_name(const char *name) {
    if (name == NULL) {
        return DEV_UNKNOWN;
    } else if (strcmp(name, "S&B VOLCANO H") == 0) {
        return DEV_VOLCANO;
    } else if (strcmp(name, "STORZ&BICKEL") == 0) {
        return DEV_CRAFTY;
    } else {
        return DEV_UNKNOWN;
    }
}

int8_t models_get_serial(const uint8_t *data, size_t data_len, char *buff, size_t buff_len) {
    if ((data == NULL) || (data_len <= 0) || (buff == NULL) || (buff_len <= 0)) {
        return -1;
    }

    const size_t serial_len = 8;
    const size_t serial_off = 2;
    if ((data_len < (serial_len + serial_off)) || (buff_len < (serial_len + 1))) {
        return -2;
    }
    memcpy(buff, data + serial_off, serial_len);
    buff[serial_len] = '\0';

    return 0;
}
