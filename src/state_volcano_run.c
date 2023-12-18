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
#include "lcd.h"
#include "volcano.h"
#include "workflow.h"
#include "util.h"
#include "state.h"
#include "state_volcano_run.h"

// only used to draw textbox, not for buttons
#include "menu.h"

static uint16_t wf_index = 0;
static bd_addr_t ble_addr = {0};
static bd_addr_type_t ble_type = 0;
static bool wait_for_connect = false;
static bool wait_for_disconnect = false;

void state_volcano_run_index(uint16_t index) {
    wf_index = index;
}

void state_volcano_run_target(bd_addr_t addr, bd_addr_type_t type) {
    debug("%s %d", bd_addr_to_str(addr), type);
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;
}

static void volcano_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        if ((!wait_for_connect) && (!wait_for_disconnect)) {
            debug("workflow abort");
            wf_reset();
            volcano_set_pump_state(false);
            volcano_set_heater_state(false);
            ble_disconnect();
            wait_for_disconnect = true;
        }
    }
}

void state_volcano_run_enter(void) {
    menu_init(NULL, NULL, NULL, NULL);
    buttons_callback(volcano_buttons);

    debug("workflow connect");
    ble_connect(ble_addr, ble_type);
    wait_for_connect = true;
}

void state_volcano_run_exit(void) {
    menu_deinit();
    wf_reset();
}

static void bar_graph(int y_off, int h, int val_min, int val, int val_max) {
    float v = map(val, val_min, val_max, 0.0f, 1.0f);
    uint16_t width = v * (LCD_WIDTH - 1);
    uint32_t c = from_hsv(v * 0.333, 1.0, 1.0);
    lcd_write_rect(0, y_off, width, y_off + h - 1, c);
}

static void draw(struct menu_state *menu) {
    static struct wf_state prev_state = {0};
    struct wf_state state = wf_status();

    if ((state.step == prev_state.step)
        && ((((state.step->op == OP_SET_TEMPERATURE) || (state.step->op == OP_WAIT_TEMPERATURE))
               && ((state.curr_val / 10) == (prev_state.curr_val / 10)))
            || (((state.step->op == OP_PUMP_TIME) || (state.step->op == OP_WAIT_TIME))
               && ((state.curr_val / 500) == (prev_state.curr_val / 500))))) {
        return;
    }
    prev_state = state;

    menu->lines = 3;
    menu->y_off = 20 + 2;

    lcd_write_rect(0, 50,
                   LCD_WIDTH - 1,
                   50 + menu->y_off - 1,
                   LCD_BLACK);
    lcd_write_rect(0, 50 + MENU_BOX_HEIGHT(3, 20, 2) + menu->y_off,
                   LCD_WIDTH - 1,
                   50 + MENU_BOX_HEIGHT(3, 20, 2) + (menu->y_off * 2) - 1,
                   LCD_BLACK);

    if (state.status == WF_IDLE) {
        if (wait_for_connect) {
            snprintf(menu->buff, MENU_MAX_LEN,
                     "Connecting\nand\nDiscovering");
        } else if (wait_for_disconnect) {
            snprintf(menu->buff, MENU_MAX_LEN,
                     "\nDisconnecting");
        } else {
            snprintf(menu->buff, MENU_MAX_LEN,
                     "\nDone");
        }
        return;
    }

    bar_graph(50, menu->y_off, 0, state.index + 1, state.count);
    bar_graph(50 + MENU_BOX_HEIGHT(3, 20, 2) + menu->y_off, menu->y_off,
              state.start_val, state.curr_val, state.step->val);

    int pos = 0;
    pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                    "step %d / %d\n", state.index, state.count);

    switch (state.step->op) {
    case OP_SET_TEMPERATURE:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "\n%s", wf_step_str(state.step));
        break;

    case OP_WAIT_TEMPERATURE:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "%s\n", wf_step_str(state.step));
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "%.1f -> %.1f -> %.1f",
                        state.start_val / 10.0f,
                        state.curr_val / 10.0f,
                        state.step->val / 10.0f);
        break;

    case OP_WAIT_TIME:
    case OP_PUMP_TIME:
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "%s\n", wf_step_str(state.step));
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                        "%.0f -> %.1f -> %.0f",
                        state.start_val / 1000.0f,
                        state.curr_val / 1000.0f,
                        state.step->val / 1000.0f);
        break;
    }
}

void state_volcano_run_run(void) {
    // start workflow when connected
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        debug("workflow start");
        wf_start(wf_index);
    }

    // visualize workflow status
    menu_run(draw, true);

    // auto disconnect when end of workflow is reached
    if ((!wait_for_connect) && (!wait_for_disconnect)) {
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
