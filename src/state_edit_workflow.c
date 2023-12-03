/*
 * state_edit_workflow.c
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
#include "mem.h"
#include "menu.h"
#include "workflow.h"
#include "state.h"
#include "state_value.h"
#include "state_edit_workflow.h"

static uint16_t wf_index = 0;

static void enter_cb(int selection) {
    static char buff[20];

    if ((selection >= 0) && (selection < wf_steps(wf_index))) {
        struct wf_step *step = wf_get_step(wf_index, selection);
        switch (step->op) {
        case OP_SET_TEMPERATURE:
        case OP_WAIT_TEMPERATURE:
            snprintf(buff, sizeof(buff),
                     "%s Temp.",
                     step->op == OP_WAIT_TEMPERATURE ? "Wait" : "Set");
            state_value_set(&step->val,
                            sizeof(step->val),
                            400, 2300, VAL_STEP_INCREMENT, 10,
                            buff);
            break;

        case OP_WAIT_TIME:
        case OP_PUMP_TIME:
            snprintf(buff, sizeof(buff),
                     "%s Time",
                     step->op == OP_WAIT_TIME ? "Wait" : "Pump");
            state_value_set(&step->val,
                            sizeof(step->val),
                            0, 60000, VAL_STEP_INCREMENT, 1000,
                            buff);
            break;
        }

        state_value_return(STATE_EDIT_WORKFLOW);
        state_switch(STATE_VALUE);
    }
}

static void lower_cb(int selection) {
    if ((selection > 0) && (selection < wf_steps(wf_index))) {
        wf_move_step_down(wf_index, selection);
    }
}

static void upper_cb(int selection) {
    if ((selection >= 0) && (selection < (wf_steps(wf_index) - 1))) {
        wf_move_step_up(wf_index, selection);
    }
}

static void exit_cb(void) {
    state_switch(STATE_SCAN);
}

void state_edit_wf_index(uint16_t index) {
    wf_index = index;
}

void state_edit_wf_enter(void) {
    menu_init(enter_cb, lower_cb, upper_cb, exit_cb);
}

void state_edit_wf_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    menu->length = wf_steps(wf_index);

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

        struct wf_step *step = wf_get_step(wf_index, i);
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "% 2d: %s\n", i, wf_step_str(step));
    }

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_edit_wf_run(void) {
    menu_run(draw, false);
}
