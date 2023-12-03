/*
 * volcano.h
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

#ifndef __VOLCANO_H__
#define __VOLCANO_H__

#include <stdint.h>
#include <stdbool.h>

#include "models.h"

enum volcano_state {
    VOLCANO_STATE_NONE = 0,
    VOLCANO_STATE_HEATER = (1 << 0),
    VOLCANO_STATE_PUMP = (1 << 1),
    VOLCANO_STATE_INVALID = 0xFF,
};

// returns < 0 on error
int8_t volcano_discover_characteristics(void);

// in 1/10th degrees C, or < 0 on error
int16_t volcano_get_current_temp(void);
int16_t volcano_get_target_temp(void);

// v in 1/10th degrees C, returns < 0 on error
int8_t volcano_set_target_temp(uint16_t v);

// returns < 0 on error
int8_t volcano_set_heater_state(bool value);
int8_t volcano_set_pump_state(bool value);
int8_t volcano_set_unit(enum unit unit);
int8_t volcano_set_vibration(bool value);
int8_t volcano_set_display_cooling(bool value);

enum unit volcano_get_unit(void);
enum volcano_state volcano_get_state(void);

// returns bool, or < 0 on error
int8_t volcano_get_vibration(void);
int8_t volcano_get_display_cooling(void);

#endif // __VOLCANO_H__
