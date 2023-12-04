/*
 * venty.h
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

#ifndef __VENTY_H__
#define __VENTY_H__

#include <stdint.h>
#include <stdbool.h>

// in 1/10th degrees C, or < 0 on error
int16_t venty_get_target_temp(void);

// v in 1/10th degrees C, returns < 0 on error
int8_t venty_set_target_temp(uint16_t value);

// returns < 0 on error
int8_t venty_set_heater_state(bool value);
int8_t venty_set_eco_current(bool value);
int8_t venty_set_eco_voltage(bool value);

// in percent, or < 0 on error
int8_t venty_get_battery_state(void);

// bool, or < 0 on error
int8_t venty_get_eco_current(void);
int8_t venty_get_eco_voltage(void);

#endif // __VENTY_H__
