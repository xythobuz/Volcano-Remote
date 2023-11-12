/*
 * state_volcano_workflow.c
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
#include "menu.h"
#include "workflow.h"
#include "state.h"
#include "state_volcano_run.h"
#include "state_volcano_workflow.h"

static void enter_cb(int selection) {
    if ((selection >= 0) && (selection < wf_count())) {
        state_volcano_run_index(selection);
        state_switch(STATE_VOLCANO_RUN);
    }
}

static void exit_cb(void) {
    state_switch(STATE_SCAN);
}

void state_volcano_wf_enter(void) {
    menu_init(enter_cb, exit_cb);
}

void state_volcano_wf_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    menu->length = wf_count();

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

        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "'%s' by %s\n", wf_name(i), wf_author(i));
    }

    if ((menu->selection < 0) && (menu->length > 0)) {
        menu->selection = 0;
    }

    if (menu->length == 0) {
        strncpy(menu->buff, "NONE", MENU_MAX_LEN);
    }
}

void state_volcano_wf_run(void) {
    menu_run(draw);
}
