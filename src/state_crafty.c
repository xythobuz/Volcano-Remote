/*
 * state_crafty.c
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
#include "crafty.h"
#include "log.h"
#include "state.h"
#include "state_crafty.h"

#include "menu.h"

#define CRAFTY_UPDATE_TIME_MS 3000

static bd_addr_t ble_addr = {0};
static bd_addr_type_t ble_type = 0;
static bool wait_for_connect = false;
static bool wait_for_disconnect = false;
static bool heater_state = false;
static int16_t target_temp = 0;

static void crafty_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        if ((!wait_for_connect) && (!wait_for_disconnect)) {
            debug("crafty disconnect");
            ble_disconnect();
            wait_for_disconnect = true;
        } else {
            debug("invalid state for disconnect");
        }
    } else if (state && (btn == BTN_A)) {
        heater_state = !heater_state;
        debug("crafty heater %s", heater_state ? "on" : "off");
        crafty_set_heater_state(heater_state);
    } else if (state && (btn == BTN_LEFT)) {
        target_temp -= 10;
        if (target_temp < 400) {
            target_temp = 400;
        } else if (target_temp > 2100) {
            target_temp = 2100;
        }
        debug("crafty temp to %.1f", target_temp / 10.0f);
        crafty_set_target_temp(target_temp);
    } else if (state && (btn == BTN_RIGHT)) {
        target_temp += 10;
        if (target_temp < 400) {
            target_temp = 400;
        } else if (target_temp > 2100) {
            target_temp = 2100;
        }
        debug("crafty temp to %.1f", target_temp / 10.0f);
        crafty_set_target_temp(target_temp);
    }
}

void state_crafty_target(bd_addr_t addr, bd_addr_type_t type) {
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;
}

void state_crafty_enter(void) {
    debug("crafty connect");
    ble_connect(ble_addr, ble_type);
    wait_for_connect = true;

    buttons_callback(crafty_buttons);
}

void state_crafty_exit(void) {
    buttons_callback(NULL);
}

static void draw(struct menu_state *menu) {
    if (wait_for_connect) {
        snprintf(menu->buff, MENU_MAX_LEN, "Connecting...");
    } else if (wait_for_disconnect) {
        snprintf(menu->buff, MENU_MAX_LEN, "Disconnecting...");
    } else {
        target_temp = crafty_get_target_temp();
        int16_t ct = crafty_get_current_temp();
        int8_t bs = crafty_get_battery_state();

        snprintf(menu->buff, MENU_MAX_LEN,
                 "Target: %.1f C\n"
                 "Current: %.1f C\n"
                 "Battery: %d %%",
                 target_temp / 10.0f, ct / 10.0f, bs);
    }
}

void state_crafty_run(void) {
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        debug("crafty start");
    }

    static uint32_t last = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (((now - last) >= CRAFTY_UPDATE_TIME_MS) || (last == 0)) {
        menu_run(draw, true);
        last = to_ms_since_boot(get_absolute_time());
    }

    if (wait_for_disconnect && !ble_is_connected()) {
        wait_for_disconnect = false;
        debug("crafty done");
        state_switch(STATE_SCAN);
    }
}
