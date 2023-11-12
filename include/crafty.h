/*
 * crafty.h
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

#ifndef __CRAFTY_H__
#define __CRAFTY_H__

#include <stdint.h>
#include <stdbool.h>

// in 1/10th degrees C, or < 0 on error
int16_t crafty_get_current_temp(void);
int16_t crafty_get_target_temp(void);

// v in 1/10th degrees C, returns < 0 on error
int8_t crafty_set_target_temp(uint16_t v);

// returns < 0 on error
int8_t crafty_set_heater_state(bool value);

// in percent, or < 0 on error
int8_t crafty_get_battery_state(void);

#endif // __CRAFTY_H__
