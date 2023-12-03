/*
 * menu.h
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

#ifndef __MENU_H__
#define __MENU_H__

#include <stdbool.h>

#define MENU_MAX_LINES 5
#define MENU_MAX_LEN (MENU_MAX_LINES * 32)

#define MENU_BOX_HEIGHT(lines, font, space) ((lines * font) + ((lines - 1) * space))

struct menu_state {
    int off;
    int selection;
    int length;
    char buff[MENU_MAX_LEN];
    int lines;
    int y_off;
};

void menu_init(void (*enter)(int),
               void (*up)(int),
               void (*down)(int),
               void (*exit)(void));
void menu_deinit(void);

void menu_run(void (*cb)(struct menu_state *), bool centered);

extern bool menu_got_input;

#define ADD_STATIC_ELEMENT(format, ...) {                                \
    menu->length += 1;                                                   \
    if (((menu->length - 1) >= menu->off)                                \
        && ((menu->length - 1 - menu->off) < menu->lines)) {             \
        if ((menu->length - 1) == menu->selection) {                     \
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "> "); \
        } else {                                                         \
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "  "); \
        }                                                                \
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,            \
                        format __VA_OPT__(,) __VA_ARGS__);               \
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "\n");     \
    }                                                                    \
}

#endif // __MENU_H__
