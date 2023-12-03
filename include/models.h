/*
 * models.h
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

#ifndef __MODELS_H__
#define __MODELS_H__

#include <stdint.h>

enum known_devices {
    DEV_UNKNOWN = 0,
    DEV_VOLCANO,
    DEV_CRAFTY,
};

enum unit {
    UNIT_C = 0,
    UNIT_F,
    UNIT_INVALID,
};

enum known_devices models_filter_name(const char *name);
int8_t models_get_serial(const uint8_t *data, size_t data_len, char *buff, size_t buff_len);

#endif // __MODELS_H__
