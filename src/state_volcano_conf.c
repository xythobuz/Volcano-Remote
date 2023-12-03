/*
 * state_volcano_conf.c
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

#include "config.h"
#include "log.h"
#include "menu.h"
#include "volcano.h"
#include "state.h"
#include "state_value.h"
#include "state_volcano_conf.h"

static bd_addr_t ble_addr = {0};
static bd_addr_type_t ble_type = 0;
static bool wait_for_connect = false;
static bool wait_for_disconnect = false;
static bool connected = false;

static bool val_celsius = false;
static bool val_vibrate = false;
static bool val_disp_cool = false;
static uint16_t val_auto_shutoff = 0;
static uint8_t val_brightness = 0;

void state_volcano_conf_target(bd_addr_t addr, bd_addr_type_t type) {
    debug("%s %d", bd_addr_to_str(addr), type);
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;

    connected = false;
}

static void enter_cb(int selection) {
    switch (selection) {
    case 0:
        // Celsius
        state_value_set(&val_celsius,
                        sizeof(val_celsius),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Celsius");
        state_value_return(STATE_VOLCANO_CONF);
        state_switch(STATE_VALUE);
        break;

    case 1:
        // Vibrate
        state_value_set(&val_vibrate,
                        sizeof(val_vibrate),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Vibrate");
        state_value_return(STATE_VOLCANO_CONF);
        state_switch(STATE_VALUE);
        break;

    case 2:
        // Disp. Cool
        state_value_set(&val_disp_cool,
                        sizeof(val_disp_cool),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Disp. Cool");
        state_value_return(STATE_VOLCANO_CONF);
        state_switch(STATE_VALUE);
        break;

    case 3:
        // Auto Shutoff
        state_value_set(&val_auto_shutoff,
                        sizeof(val_auto_shutoff),
                        0, 60 * 60, VAL_STEP_INCREMENT, 60,
                        "Auto Shutoff");
        state_value_return(STATE_VOLCANO_CONF);
        state_switch(STATE_VALUE);
        break;

    case 4:
        // Brightness
        state_value_set(&val_brightness,
                        sizeof(val_brightness),
                        0, 100, VAL_STEP_INCREMENT, 10,
                        "Brightness");
        state_value_return(STATE_VOLCANO_CONF);
        state_switch(STATE_VALUE);
        break;
    }
}

static void send_values(void) {
    volcano_set_unit(val_celsius ? UNIT_C : UNIT_F);
    sleep_ms(150);
    volcano_set_vibration(val_vibrate);
    sleep_ms(150);
    volcano_set_display_cooling(val_disp_cool);
    sleep_ms(150);
    volcano_set_auto_shutoff(val_auto_shutoff);
    sleep_ms(150);
    volcano_set_brightness(val_brightness);
}

static void fetch_values(void) {
    volcano_discover_characteristics(false, true);

    enum unit unit = volcano_get_unit();
    val_celsius = (unit == UNIT_C);

    int8_t r = volcano_get_vibration();
    val_vibrate = (r == 1);

    r = volcano_get_display_cooling();
    val_disp_cool = (r == 1);

    val_auto_shutoff = volcano_get_auto_shutoff();

    val_brightness = volcano_get_brightness();
}

static void exit_cb(void) {
    debug("volcano disconnect");
    ble_disconnect();
    wait_for_disconnect = true;
}

void state_volcano_conf_enter(void) {
    menu_init(enter_cb, NULL, NULL, exit_cb);

    if (!connected) {
        debug("volcano connect");
        ble_connect(ble_addr, ble_type);
        wait_for_connect = true;
    } else {
        debug("volcano write");
        send_values();
    }
}

void state_volcano_conf_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    if (wait_for_connect) {
        snprintf(menu->buff, MENU_MAX_LEN,
                 "Connecting\nand\nDiscovering");
        return;
    } else if (wait_for_disconnect) {
        snprintf(menu->buff, MENU_MAX_LEN,
                 "\nDisconnecting");
        return;
    }

    int pos = 0;
    menu->length = 0;

    ADD_STATIC_ELEMENT("Celsius");
    ADD_STATIC_ELEMENT("Vibrate");
    ADD_STATIC_ELEMENT("Disp. Cool");
    ADD_STATIC_ELEMENT("Auto Shutoff");
    ADD_STATIC_ELEMENT("Brightness");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_volcano_conf_run(void) {
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        connected = true;
        debug("volcano start");
        fetch_values();
    }

    menu_run(draw, true);

    // back to main menu when disconnected
    if (wait_for_disconnect && !ble_is_connected()) {
        wait_for_disconnect = false;
        connected = false;
        debug("volcano done");
        state_switch(STATE_SCAN);
    }
}
