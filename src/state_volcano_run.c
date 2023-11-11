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
#include "workflow.h"
#include "state.h"
#include "state_volcano_run.h"

#include "menu.h"

static uint16_t wf_index;
static bd_addr_t ble_addr;
static bd_addr_type_t ble_type;
static bool wait_for_connect = false;

void state_volcano_run_index(uint16_t index) {
    wf_index = index;
}

void state_volcano_run_target(bd_addr_t addr, bd_addr_type_t type) {
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;
}

void state_volcano_run_enter(void) {
    ble_connect(ble_addr, ble_type);
    wait_for_connect = true;
}

void state_volcano_run_exit(void) {
    wf_reset();
    ble_disconnect();
}

static void draw(struct menu_state *menu) {
    struct wf_state state = wf_status();
    snprintf(menu->buff, MENU_MAX_LEN, "%d / %d", state.step, state.count);
}

void state_volcano_run_run(void) {
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        wf_start(wf_index);
    }

    menu_run(draw);

    struct wf_state state = wf_status();
    if (state.status == WF_IDLE) {
        state_switch(STATE_SCAN);
    }
}
