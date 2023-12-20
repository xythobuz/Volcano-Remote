/*
 * state_about.c
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
#include "log.h"
#include "lcd.h"
#include "text.h"
#include "textbox.h"
#include "menu.h"
#include "state.h"
#include "state_about.h"

#define xstr(s) str(s)
#define str(s) #s

static const char *about_text =
    "Volcano RC Gadget\n"
    "by xythobuz\n"
    "Licensed as GPLv3\n"
    "\n"

    "V" xstr(APP_VERSION_MAJOR) "." xstr(APP_VERSION_MINOR) "\n"
#ifdef NDEBUG
    "Release Build\n"
#else // NDEBUG
    "Debug Build\n"
#endif // NDEBUG
    __DATE__ " " __TIME__ "\n"
    "\n"

    "Included libs:\n"
    "hathach/tinyusb\n"
    "abbrev/fatfs\n"
    "bluekitchen/btstack\n"
    "mcufont/mcufont\n"
    "hepingood/st7789\n"
    "usedbytes/picowota\n"
    "lwip-tcpip/lwip\n"
    "micropython/dhcpserver\n"
    "\n"

    "This program is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n"
    "\n"
    "See <http://www.gnu.org/licenses/>.\n"
;

static const uint16_t step_size = 10;
static const uint16_t max_height = MENU_BOX_HEIGHT(MENU_MAX_LINES, 20, 2);

static uint16_t off = 0;
static bool held_up = false;
static bool held_down = false;
static int16_t last_draw_off = 0;

static void draw(void) {
    int16_t r;
    r = text_box(about_text, false,
                 "DejaVuSerif16",
                 0, LCD_WIDTH,
                 50, max_height,
                 -off);
    last_draw_off = r;
}

static void about_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        state_switch(STATE_SCAN);
    } else if (btn == BTN_UP) {
        held_up = state;
    } else if (btn == BTN_DOWN) {
        held_down = state;
    }
}

void state_about_enter(void) {
    buttons_callback(about_buttons);
    off = 0;
    draw();
}

void state_about_exit(void) {
    buttons_callback(NULL);
}

void state_about_run(void) {
    if (held_up) {
        if (off >= step_size) {
            off -= step_size;
            draw();
        }
    }
    if (held_down) {
        if (last_draw_off >= max_height) {
            off += step_size;
            draw();
        }
    }
}
