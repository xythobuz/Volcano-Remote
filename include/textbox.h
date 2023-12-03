/*
 * textbox.h
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

#ifndef __TEXTBOX_H__
#define __TEXTBOX_H__

int16_t text_box(const char *s, bool centered,
                 const char *fontname,
                 uint16_t x_off, uint16_t width,
                 uint16_t y_off, uint16_t height,
                 int16_t y_text_off);

#endif // __TEXTBOX_H__
