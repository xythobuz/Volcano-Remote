/*
 * image.h
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "pico/stdlib.h"

void image_draw(char *data, uint width, uint height);

void draw_splash(void);
void draw_battery_indicator(void);
void battery_run(void);

#endif // __IMAGE_H__
