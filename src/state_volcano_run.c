/*
 * state_volcano_run.c
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
#include "volcano.h"
#include "workflow.h"
#include "state.h"
#include "state_volcano_run.h"

// only used to draw textbox, not for buttons
#include "menu.h"

static uint16_t wf_index = 0;
static bd_addr_t ble_addr = {0};
static bd_addr_type_t ble_type = 0;
static bool wait_for_connect = false;
static bool wait_for_disconnect = false;
static bool aborted = false;

void state_volcano_run_index(uint16_t index) {
    wf_index = index;
}

void state_volcano_run_target(bd_addr_t addr, bd_addr_type_t type) {
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;
}

static void volcano_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        if ((!wait_for_connect) && (!wait_for_disconnect)) {
            debug("workflow abort");
            aborted = true;
            wf_reset();
            volcano_set_pump_state(false);
            volcano_set_heater_state(false);
            ble_disconnect();
            wait_for_disconnect = true;
        }
    }
}

void state_volcano_run_enter(void) {
    menu_init(NULL, NULL);
    buttons_callback(volcano_buttons);

    debug("workflow connect");
    ble_connect(ble_addr, ble_type);
    wait_for_connect = true;

    aborted = false;
}

void state_volcano_run_exit(void) {
    menu_deinit();
    wf_reset();
}

static void draw(struct menu_state *menu) {
    struct wf_state state = wf_status();

    int pos = 0;
    pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                    "step %d / %d\n\n", state.index, state.count);

    switch (state.step.op) {
    case OP_SET_TEMPERATURE:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "set temp %.1f C", state.step.val / 10.0f);
        break;

    case OP_WAIT_TEMPERATURE:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "wait temp %.1f C", state.step.val / 10.0f);
        break;

    case OP_WAIT_TIME:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "wait time %.1f s", state.step.val / 1000.0f);
        break;

    case OP_PUMP_TIME:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "pump time %.1f s", state.step.val / 1000.0f);
        break;
    }

    // TODO visualize
}

void state_volcano_run_run(void) {
    // start workflow when connected
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        debug("workflow start");
        wf_start(wf_index);
    }

    // visualize workflow status
    menu_run(draw);

    // auto disconnect when end of workflow is reached
    if ((!wait_for_connect) && (!aborted)) {
        struct wf_state state = wf_status();
        if (state.status == WF_IDLE) {
            debug("workflow disconnect");
            ble_disconnect();
            wait_for_disconnect = true;
        }
    }

    // back to main menu when disconnected
    if (wait_for_disconnect && !ble_is_connected()) {
        wait_for_disconnect = false;
        debug("workflow done");
        state_switch(STATE_SCAN);
    }
}