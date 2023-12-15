/*
 * state_venty.c
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
#include "venty.h"
#include "log.h"
#include "state.h"
#include "state_value.h"
#include "state_venty.h"

static bd_addr_t ble_addr = {0};
static bd_addr_type_t ble_type = 0;
static bool wait_for_connect = false;
static bool wait_for_disconnect = false;
static bool connected = false;

static bool heater_state = false;
static int16_t target_temp = 0;
static int8_t battery = 0;
static int8_t eco_current = 0;
static int8_t eco_voltage = 0;
static int8_t vibration = 0;
static int8_t brightness = 0;

void state_venty_target(bd_addr_t addr, bd_addr_type_t type) {
    memcpy(ble_addr, addr, sizeof(bd_addr_t));
    ble_type = type;

    connected = false;
}

static void exit_cb(void) {
    debug("venty disconnect");
    ble_disconnect();
    wait_for_disconnect = true;
}

static void enter_cb(int selection) {
    switch (selection) {
    case 0:
        // Battery
        break;

    case 1:
        // Target
        state_value_set(&target_temp,
                        sizeof(target_temp),
                        400, 2100, VAL_STEP_INCREMENT, 10,
                        "Target");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    case 2:
        // Heater
        state_value_set(&heater_state,
                        sizeof(heater_state),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Heater");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    case 3:
        // Eco Current
        state_value_set(&eco_current,
                        sizeof(eco_current),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Eco Amps");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    case 4:
        // Eco Voltage
        state_value_set(&eco_voltage,
                        sizeof(eco_voltage),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Eco Volt");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    case 5:
        // Vibration
        state_value_set(&vibration,
                        sizeof(vibration),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Vibration");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    case 6:
        // Brightness
        state_value_set(&brightness,
                        sizeof(brightness),
                        1, 9, VAL_STEP_INCREMENT, 1,
                        "Brightness");
        state_value_return(STATE_VENTY);
        state_switch(STATE_VALUE);
        break;

    default:
        exit_cb();
        break;
    }
}

static void send_values(void) {
    venty_set_heater_state(heater_state);
    sleep_ms(250);
    venty_set_target_temp(target_temp);
    sleep_ms(250);
    venty_set_eco_current(eco_current);
    sleep_ms(250);
    venty_set_eco_voltage(eco_voltage);
    sleep_ms(250);
    venty_set_vibration(vibration);
    sleep_ms(250);
    venty_set_brightness(brightness);
}

static void fetch_values(void) {
    heater_state = venty_get_heater_state();
    target_temp = venty_get_target_temp();
    battery = venty_get_battery_state();
    eco_current = venty_get_eco_current();
    eco_voltage = venty_get_eco_voltage();
    vibration = venty_get_vibration();
    brightness = venty_get_brightness();
}

void state_venty_enter(void) {
    menu_init(enter_cb, NULL, NULL, exit_cb);

    if (!connected) {
        debug("venty connect");
        ble_connect(ble_addr, ble_type);
        wait_for_connect = true;
    } else {
        debug("venty write");
        send_values();
    }
}

void state_venty_exit(void) {
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    if (wait_for_connect) {
        snprintf(menu->buff, MENU_MAX_LEN,
                 "\nConnecting");
        return;
    } else if (wait_for_disconnect) {
        snprintf(menu->buff, MENU_MAX_LEN,
                 "\nDisconnecting");
        return;
    }

    int pos = 0;
    menu->length = 0;

    ADD_STATIC_ELEMENT("Battery: %d %%", battery);
    ADD_STATIC_ELEMENT("Target (%d C)", target_temp / 10);
    ADD_STATIC_ELEMENT("Heater (%d)", heater_state);
    ADD_STATIC_ELEMENT("Eco Amps (%d)", eco_current);
    ADD_STATIC_ELEMENT("Eco Volt (%d)", eco_voltage);
    ADD_STATIC_ELEMENT("Vibration (%d)", vibration);
    ADD_STATIC_ELEMENT("Brightness (%d)", brightness);

    ADD_STATIC_ELEMENT("... go back");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_venty_run(void) {
    if (wait_for_connect && ble_is_connected()) {
        wait_for_connect = false;
        connected = true;
        debug("venty start");
        fetch_values();
    }

    menu_run(draw, true);

    if (wait_for_disconnect && !ble_is_connected()) {
        wait_for_disconnect = false;
        connected = false;
        debug("venty done");
        state_switch(STATE_SCAN);
    }
}
