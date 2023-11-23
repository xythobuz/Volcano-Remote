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
#include "state_edit_workflow.h"
#include "state_volcano_workflow.h"

static bool edit_mode = false;

static void enter_cb(int selection) {
    if ((selection >= 0) && (selection < wf_count())) {
        if (edit_mode) {
            state_edit_wf_index(selection);
            state_switch(STATE_EDIT_WORKFLOW);
        } else {
            state_volcano_run_index(selection);
            state_switch(STATE_VOLCANO_RUN);
        }
    }
}

static void lower_cb(int selection) {
    if (!edit_mode) {
        return;
    }

    if ((selection > 0) && (selection < wf_count())) {
        wf_move_down(selection);
    }
}

static void upper_cb(int selection) {
    if (!edit_mode) {
        return;
    }

    if ((selection >= 0) && (selection < (wf_count() - 1))) {
        wf_move_up(selection);
    }
}

static void exit_cb(void) {
    state_switch(STATE_SCAN);
}

void state_volcano_wf_edit(bool edit) {
    edit_mode = edit;
}

void state_volcano_wf_enter(void) {
    if (edit_mode) {
        menu_init(enter_cb, lower_cb, upper_cb, exit_cb);
    } else {
        menu_init(enter_cb, NULL, NULL, exit_cb);
    }
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

        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "'%s' by %s\n", wf_name(i), wf_author(i));
    }

    if ((menu->selection < 0) && (menu->length > 0)) {
        menu->selection = 0;
    }

    if (menu->length == 0) {
        strncpy(menu->buff, "NONE", MENU_MAX_LEN);
    }
}

void state_volcano_wf_run(void) {
    menu_run(draw, false);
}
