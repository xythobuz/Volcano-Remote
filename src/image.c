/*
 * image.c
 *
 * Copyright (c) 2022 - 2023 Thomas Buck (thomas@xythobuz.de)
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

#include "pico/stdlib.h"

#include "config.h"
#include "lcd.h"
#include "image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtrigraphs"
#include "logo.h"
#pragma GCC diagnostic pop

void draw_splash(void) {
    char *data = logo_rgb_data;
    for (uint x = 0; x < logo_width; x++) {
        for (uint y = 0; y < logo_height; y++) {
            uint32_t pixel[3];
            HEADER_PIXEL(data, pixel);

            uint32_t color = (pixel[0] >> 3) << 11;
            color |= (pixel[1] >> 2) << 5;
            color |= pixel[2] >> 3;
            lcd_write_point(240 - x - 1, y, color);
        }
    }
}
