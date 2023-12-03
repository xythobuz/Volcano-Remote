/*
 * menu.c
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
#include "buttons.h"
#include "text.h"
#include "lcd.h"
#include "menu.h"

static char prev_buff[MENU_MAX_LEN] = {0};
static struct menu_state menu = { .off = 0, .selection = -1, .length = 0, .buff = {0} };
static void (*enter_callback)(int) = NULL;
static void (*up_callback)(int) = NULL;
static void (*down_callback)(int) = NULL;
static void (*exit_callback)(void) = NULL;

bool menu_got_input = false;

static void menu_buttons(enum buttons btn, bool state) {
    menu_got_input = true;

    if (state && ((btn == BTN_ENTER) || (btn == BTN_A))) {
        if (enter_callback) {
            enter_callback(menu.selection);
        }
        return;
    } else if (state && (btn == BTN_B)) {
        if (up_callback) {
            up_callback(menu.selection);
        }
        return;
    } else if (state && (btn == BTN_X)) {
        if (down_callback) {
            down_callback(menu.selection);
        }
        return;
    } else if (state && (btn == BTN_Y)) {
        if (exit_callback) {
            exit_callback();
        }
    } else if ((!state) || ((btn != BTN_UP) && (btn != BTN_DOWN))) {
        return;
    }

    if (state && (btn == BTN_UP)) {
        if (menu.selection < 0) {
            menu.selection = menu.length - 1;
        } else {
            menu.selection -= 1;
        }
    } else if (state && (btn == BTN_DOWN)) {
        if (menu.selection < 0) {
            menu.selection = 0;
        } else {
            menu.selection += 1;
        }
    }

    if (menu.selection < 0) {
        menu.selection += menu.length;
    }
    if (menu.selection >= menu.length) {
        menu.selection -= menu.length;
    }
    if (menu.selection >= 0) {
        while (menu.selection < menu.off) {
            menu.off -= 1;
        }
        while (menu.selection >= (menu.off + MENU_MAX_LINES)) {
            menu.off += 1;
        }
    }
}

void menu_init(void (*enter)(int),
               void (*up)(int),
               void (*down)(int),
               void (*exit)(void)) {
    menu.off = 0;
    menu.selection = -1;
    menu.length = 0;

    enter_callback = enter;
    up_callback = up;
    down_callback = down;
    exit_callback = exit;
    buttons_callback(menu_buttons);
}

void menu_deinit(void) {
    buttons_callback(NULL);
}

void menu_run(void (*draw)(struct menu_state *), bool centered) {
    if (draw) {
        draw(&menu);
    }

    if (strncmp(menu.buff, prev_buff, MENU_MAX_LEN) != 0) {
        strncpy(prev_buff, menu.buff, MENU_MAX_LEN);
        text_box(menu.buff, centered,
                 "fixed_10x20",
                 0, LCD_WIDTH,
                 50, TEXT_BOX_HEIGHT(20, 2),
                 0);
    }
}
