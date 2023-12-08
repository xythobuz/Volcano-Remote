/*
 * state_wifi_edit.c
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

#include <stddef.h>
#include <stdio.h>

#include "config.h"
#include "menu.h"
#include "mem.h"
#include "state.h"
#include "state_string.h"
#include "state_wifi_edit.h"

static uint16_t wifi_index = 0;

static void exit_cb(void) {
    state_switch(STATE_SETTINGS);
}

static void enter_cb(int selection) {
    switch (selection) {
    case 0:
        // SSID
        state_string_set(mem_data()->net[wifi_index].name,
                         WIFI_MAX_NAME_LEN, "SSID");
        state_string_return(STATE_WIFI_EDIT);
        state_switch(STATE_STRING);
        break;

    case 1:
        // Password
        state_string_set(mem_data()->net[wifi_index].pass,
                         WIFI_MAX_NAME_LEN, "Password");
        state_string_return(STATE_WIFI_EDIT);
        state_switch(STATE_STRING);
        break;

    default:
        exit_cb();
        break;
    }
}

void state_wifi_edit_index(uint16_t index) {
    wifi_index = index;
}

void state_wifi_edit_enter(void) {
    menu_init(enter_cb, NULL, NULL, exit_cb);
}

void state_wifi_edit_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    int pos = 0;
    menu->length = 0;

    ADD_STATIC_ELEMENT("SSID: '%s'", mem_data()->net[wifi_index].name);
    ADD_STATIC_ELEMENT("Pass: '%s'", mem_data()->net[wifi_index].pass);

    ADD_STATIC_ELEMENT("... go back");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_wifi_edit_run(void) {
    menu_run(draw, false);
}
