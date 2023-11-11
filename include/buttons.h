/*
 * buttons.h
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

#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdbool.h>

enum buttons {
    BTN_A = 0,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_ENTER,
    NUM_BTNS // count
};

void buttons_init(void);
void buttons_callback(void (*fp)(enum buttons, bool));
void buttons_run(void);

#endif // __BUTTONS_H__

