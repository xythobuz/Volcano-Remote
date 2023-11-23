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

struct menu_state {
    int off;
    int selection;
    int length;
    char *buff;
};

void menu_init(void (*enter)(int),
               void (*up)(int),
               void (*down)(int),
               void (*exit)(void));
void menu_deinit(void);

void menu_run(void (*cb)(struct menu_state *), bool centered);

#endif // __MENU_H__
