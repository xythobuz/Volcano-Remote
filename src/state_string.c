/*
 * state_string.c
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

#include <stdio.h>

#include "config.h"
#include "buttons.h"
#include "lcd.h"
#include "menu.h"
#include "text.h"
#include "textbox.h"
#include "state_string.h"

static char *str_p = NULL;
static size_t str_len = 0;
static const char *str_name = NULL;
static enum system_state str_ret_state = STATE_SCAN;

void state_string_set(char *value, size_t length,
                      const char *name) {
    str_p = value;
    str_len = length;
    str_name = name;
}

void state_string_return(enum system_state state) {
    str_ret_state = state;
}

static void draw(void) {
    static char buff[42];

    if ((str_p == NULL) || (str_len <= 0) || (str_name == NULL)) {
        snprintf(buff, sizeof(buff), "error");
    } else {
        snprintf(buff, sizeof(buff), "%s:\n\n%s", str_name, str_p);
    }

    text_box(buff, true,
             "fixed_10x20",
             0, LCD_WIDTH,
             50, MENU_BOX_HEIGHT(MENU_MAX_LINES, 20, 2),
             0);
}

static void string_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        state_switch(str_ret_state);
    } else if (state && (btn == BTN_LEFT)) {
        // TODO
    } else if (state && (btn == BTN_RIGHT)) {
        // TODO
    } else if (state && (btn == BTN_UP)) {
        // TODO
    } else if (state && (btn == BTN_DOWN)) {
        // TODO
    }
}

void state_string_enter(void) {
    buttons_callback(string_buttons);
    draw();
}

void state_string_exit(void) {
    buttons_callback(NULL);
}

void state_string_run(void) { }
