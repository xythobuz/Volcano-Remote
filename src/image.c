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

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "lcd.h"
#include "text.h"
#include "lipo.h"
#include "util.h"
#include "image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtrigraphs"
#include "logo.h"
#pragma GCC diagnostic pop

#define BATT_INTERVAL_MS 777

void image_draw(const char *data, uint width, uint height) {
    for (uint y = 0; y < height; y++) {
        for (uint x = 0; x < width; x++) {
            uint32_t pixel[3];
            HEADER_PIXEL(data, pixel);

            uint32_t color = RGB_565(pixel[0], pixel[1], pixel[2]);
            lcd_write_point(x, y, color);
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
        .fg = RGB_565(0xFF, 0xFF, 0xFF),
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font_big,
    };
    text_draw(&text1);

    struct text_conf text2 = {
        .text = __DATE__ " @ " __TIME__,
        .x = 0,
        .y = 195,
        .justify = false,
        .alignment = MF_ALIGN_CENTER,
        .width = 240,
        .height = 240,
        .margin = 2,
        .fg = RGB_565(0xFF, 0xFF, 0xFF),
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font_small,
    };
    text_draw(&text2);
}

void draw_battery_indicator(void) {
    static const float batt_warn_limit = 15.0f;
    static char prev_s[30] = {0};
    static uint32_t prev_c = 0;

    char s[30] = {0};
    float v = lipo_voltage();
    uint32_t c = RGB_565(0xFF, 0x00, 0x00);
    if (lipo_charging()) {
        //                     "Batt:   99.9%   (4.20V)"
        snprintf(s, sizeof(s), "Batt:     Charging     ");
        c = RGB_565(0xFF, 0xFF, 0x00);
    } else {
        float percentage = lipo_percentage(v);
        snprintf(s, sizeof(s), "Batt:   %02.1f%%   (%.2fV)", percentage, v);

        if (percentage > batt_warn_limit) {
            float hue = map(percentage, batt_warn_limit, 100, 0.0, 0.333);
            c = from_hsv(hue, 1.0, 1.0);
        }
    }

    // only re-draw battery indicator when it has changed
    if ((strcmp(s, prev_s) == 0) && (prev_c == c)) {
        return;
    }
    strcpy(prev_s, s);
    prev_c = c;

    static struct text_font font = {
        .fontname = "fixed_10x20",
        .font = NULL,
    };
    if (font.font == NULL) {
        text_prepare_font(&font);
    }

    struct text_conf text = {
        .text = s,
        .x = 0,
        .y = 219,
        .justify = false,
        .alignment = MF_ALIGN_CENTER,
        .width = 240,
        .height = 240,
        .margin = 2,
        .fg = c,
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font,
    };
    text_draw(&text);
}

void battery_run(void) {
    static uint32_t last_run = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now >= (last_run + BATT_INTERVAL_MS)) || (last_run == 0)) {
        last_run = now;
        draw_battery_indicator();
    }
}
