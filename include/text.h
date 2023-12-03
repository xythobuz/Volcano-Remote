/*
 * text.h
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include "mcufont.h"
#include "menu.h"

#define TEXT_BG_NONE -1
#define TEXT_BOX_HEIGHT(font, space) ((MENU_MAX_LINES * font) + ((MENU_MAX_LINES - 1) * space))

struct text_font {
    const char *fontname;
    //int scale; // TODO not supported - requires somewhere to store scaled versions

    const struct mf_font_s *font;
};

struct text_conf {
    const char *text;
    int x;
    int y;
    int y_text_off;
    bool justify;
    enum mf_align_t alignment;
    int width;
    int height;
    int margin;
    int fg;
    int bg;

    struct text_font *font;
};

void text_prepare_font(struct text_font *tf);
int16_t text_draw(struct text_conf *tc);

int16_t text_box(const char *s, bool centered,
                 const char *fontname,
                 uint16_t x_off, uint16_t width,
                 uint16_t y_off, uint16_t height,
                 int16_t y_text_off);

#endif // __TEXT_H__
