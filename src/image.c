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

#include "config.h"
#include "lcd.h"
#include "text.h"
#include "image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtrigraphs"
#include "logo.h"
#pragma GCC diagnostic pop

void image_draw(char *data, uint width, uint height) {
    for (uint x = 0; x < width; x++) {
        for (uint y = 0; y < height; y++) {
            uint32_t pixel[3];
            HEADER_PIXEL(data, pixel);

            uint32_t color = RGB_565(pixel[0], pixel[1], pixel[2]);
            lcd_write_point(240 - x - 1, y, color);
        }
    }
}

void draw_splash(void) {
    image_draw(logo_rgb_data, logo_width, logo_height);

    struct text_font font_big = {
        .fontname = "DejaVuSerif32",
    };
    text_prepare_font(&font_big);

    struct text_font font_small = {
        .fontname = "DejaVuSerif16",
    };
    text_prepare_font(&font_small);

    struct text_conf text1 = {
        .text = "xythobuz.de",
        .x = 0,
        .y = 0,
        .justify = false,
        .alignment = MF_ALIGN_CENTER,
        .width = 240,
        .height = 240,
        .margin = 2,
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font_big,
    };
    text_draw(&text1);

    struct text_conf text2 = {
        .text = __DATE__ " " __TIME__,
        .x = 0,
        .y = 195,
        .justify = false,
        .alignment = MF_ALIGN_CENTER,
        .width = 240,
        .height = 240,
        .margin = 2,
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font_small,
    };
    text_draw(&text2);
}
