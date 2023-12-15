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
#include <string.h>

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
static size_t edit = 0, offset = 0;

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
        snprintf(buff, sizeof(buff), "%s:\n\n'%s'", str_name, str_p + offset);
    }

    text_box(buff, false,
             "fixed_10x20",
             0, LCD_WIDTH,
             50, MENU_BOX_HEIGHT(MENU_MAX_LINES, 20, 2),
             0);

    size_t ch = edit - offset + 1;
    lcd_write_rect(ch * 10,
                   50 + 3 * 22,
                   ch * 10 + 10,
                   50 + 3 * 22 + 3,
                   LCD_WHITE);
}

static void string_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        state_switch(str_ret_state);
    } else if (state && (btn == BTN_LEFT)) {
        if (edit > 0) {
            edit--;
        }
    } else if (state && (btn == BTN_RIGHT)) {
        if (edit < (str_len - 1)) {
            edit++;
        }
        size_t l = strlen(str_p);
        while (edit >= l) {
            str_p[l++] = ' ';
        }
    } else if (state && (btn == BTN_UP)) {
        char *c = str_p + edit;
        if ((*c >= ' ') && (*c < '~')) {
            (*c)++;
        }
    } else if (state && (btn == BTN_DOWN)) {
        char *c = str_p + edit;
        if ((*c > ' ') && (*c <= '~')) {
            (*c)--;
        }
    } else if (state && (btn == BTN_B)) {
        char *c = str_p + edit;
        *c = '\0';
    } else {
        return;
    }

    while (edit < offset) {
        offset -= 1;
    }
    while (edit >= (offset + (LCD_WIDTH / 10 - 3))) {
        offset += 1;
    }

    draw();
}

void state_string_enter(void) {
    buttons_callback(string_buttons);
    edit = 0;
    offset = 0;
    draw();
}

void state_string_exit(void) {
    buttons_callback(NULL);
}

void state_string_run(void) { }
