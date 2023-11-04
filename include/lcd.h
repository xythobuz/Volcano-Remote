/*
 * lipo.h
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

#ifndef __LCD_H__
#define __LCD_H__

#include <stdint.h>

#define RGB_565(r, g, b) ( \
      (((r) >> 3) << 11)   \
    | (((g) >> 2) << 5)    \
    |  ((b) >> 3)          \
)
#define RGB_565_REV(c)            \
    ((c & 0xF800) >> 8) / 255.0f, \
    ((c & 0x07E0) >> 3) / 255.0f, \
    ((c & 0x001F) << 3) / 255.0f
uint32_t from_hsv(float h, float s, float v);

void lcd_init(void);

uint16_t lcd_get_backlight(void);
void lcd_set_backlight(uint16_t value);

void lcd_clear(void);
void lcd_write_point(uint16_t x, uint16_t y, uint32_t color);
void lcd_write_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t color);

#endif // __LCD_H__
