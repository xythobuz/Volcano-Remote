/*
 * state_settings.c
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
#include "log.h"
#include "menu.h"
#include "mem.h"
#include "state.h"
#include "state_settings.h"

static void enter_cb(int selection) {
    switch (selection) {
    case 0:
        // Auto Connect
        break;

    case 1:
        // Brightness
        break;

    case 2:
        // Workflows
        break;

    case 3:
        // Reset
        mem_load_defaults();
        break;
    }
}

static void exit_cb(void) {
    state_switch(STATE_SCAN);
}

void state_settings_enter(void) {
    menu_init(enter_cb, NULL, NULL, exit_cb);
}

void state_settings_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    int pos = 0;
    menu->length = 0;

    ADD_STATIC_ELEMENT("Auto Connect");
    ADD_STATIC_ELEMENT("Brightness");
    ADD_STATIC_ELEMENT("Workflows");
    ADD_STATIC_ELEMENT("Reset");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_settings_run(void) {
    menu_run(draw, false);
}
