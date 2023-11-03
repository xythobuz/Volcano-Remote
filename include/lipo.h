/*
 * lipo.h
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/adc/read_vsys/power_status.c
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

#ifndef __LIPO_H__
#define __LIPO_H__

bool lipo_charging(void);
float lipo_voltage(void);
float lipo_percentage(float voltage);

#endif // __LIPO_H__
