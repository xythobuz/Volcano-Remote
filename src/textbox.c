/*
 * textbox.c
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

#include <string.h>

#include "config.h"
#include "lcd.h"
#include "text.h"
#include "textbox.h"

int16_t text_box(const char *s, bool centered,
                 const char *fontname,
                 uint16_t x_off, uint16_t width,
                 uint16_t y_off, uint16_t height,
                 int16_t y_text_off) {
    static struct text_font font = {
        .fontname = "",
        .font = NULL,
    };

    if ((font.font == NULL) || (strcmp(font.fontname, fontname) != 0)) {
        font.fontname = fontname;
        text_prepare_font(&font);
    }

    struct text_conf text = {
        .text = s,
        .x = x_off,
        .y = y_off,
        .y_text_off = y_text_off,
        .justify = false,
        .alignment = centered ? MF_ALIGN_CENTER : MF_ALIGN_LEFT,
        .width = width,
        .height = height,
        .margin = 2,
        .fg = LCD_WHITE,
        .bg = TEXT_BG_NONE,
        .font = &font,
    };

    lcd_write_rect(x_off, y_off,
                   x_off + width - 1,
                   y_off + height - 1,
                   LCD_BLACK);

    return text_draw(&text);
}
