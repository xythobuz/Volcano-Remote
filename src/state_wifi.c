/*
 * state_wifi.c
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

// TODO adding and removing whole networks

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "mem.h"
#include "menu.h"
#include "state.h"
#include "state_wifi_edit.h"
#include "state_wifi.h"

static void exit_cb(void) {
    state_switch(STATE_SETTINGS);
}

static void enter_cb(int selection) {
    if ((selection >= 0) && (selection < mem_data()->net_count)) {
        state_wifi_edit_index(selection);
        state_switch(STATE_WIFI_EDIT);
    } else {
        exit_cb();
    }
}

static void wifi_move_down(uint16_t index) {
    if ((index < 1) || (index >= mem_data()->net_count)) {
        return;
    }

    struct net_credentials tmp = mem_data()->net[index - 1];
    mem_data()->net[index - 1] = mem_data()->net[index];
    mem_data()->net[index] = tmp;
}

static void wifi_move_up(uint16_t index) {
    if (index >= (mem_data()->net_count - 1)) {
        return;
    }

    struct net_credentials tmp = mem_data()->net[index + 1];
    mem_data()->net[index + 1] = mem_data()->net[index];
    mem_data()->net[index] = tmp;
}

static void lower_cb(int selection) {
    if ((selection > 0) && (selection < mem_data()->net_count)) {
        wifi_move_down(selection);
        selection--;
    }
}

static void upper_cb(int selection) {
    if ((selection >= 0) && (selection < (mem_data()->net_count - 1))) {
        wifi_move_up(selection);
        selection++;
    }
}

void state_wifi_enter(void) {
    menu_init(enter_cb, lower_cb, upper_cb, exit_cb);
}

void state_wifi_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    menu->length = mem_data()->net_count;

    int pos = 0;
    for (uint16_t i = 0; i < menu->length; i++) {
        if ((i < menu->off)
            || ((i - menu->off) >= MENU_MAX_LINES)) {
            continue;
        }

        if (i == menu->selection) {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "> ");
        } else {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "  ");
        }

        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "'%s'\n", mem_data()->net[i].name);
    }

    ADD_STATIC_ELEMENT("... go back");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_wifi_run(void) {
    menu_run(draw, false);
}
