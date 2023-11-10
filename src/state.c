/*
 * state.c
 *
 * Copyright (c) 2022 - 2023 Thomas Buck (thomas@xythobuz.de)
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
#include "buttons.h"
#include "ble.h"
#include "lcd.h"
#include "text.h"
#include "state.h"

static enum system_state state = STATE_INIT;

void state_switch(enum system_state next) {
    if (state == next) {
        return;
    }

    // clean up old state when leaving it
    switch (state) {
    case STATE_SCAN:
        debug("leaving STATE_SCAN");
        ble_scan(BLE_SCAN_OFF);
        break;

    default:
        break;
    }

    // prepare new state on entering
    switch (next) {
    case STATE_SCAN:
        debug("entering STATE_SCAN");
        ble_scan(BLE_SCAN_ON);
        break;

    default:
        break;
    }

    state = next;
}

void state_run(void) {
    // TODO only for testing
    static uint32_t last_heartbeat = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now < (last_heartbeat + 1000)) {
        return;
    }
    last_heartbeat = now;

    static struct text_font font = {
        .fontname = "fixed_10x20",
        .font = NULL,
    };
    if (font.font == NULL) {
        text_prepare_font(&font);
    }

    struct text_conf text = {
        .text = "",
        .x = 0,
        .y = 50,
        .justify = false,
        .alignment = MF_ALIGN_CENTER,
        .width = 240,
        .height = 240 - 80,
        .margin = 2,
        .fg = RGB_565(0xFF, 0xFF, 0xFF),
        .bg = RGB_565(0x00, 0x00, 0x00),
        .font = &font,
    };

    switch (state) {
    case STATE_INIT:
        break;

    case STATE_SCAN: {
        struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
        int n = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);
        if (n <= 0) {
            text.text = "N\nO\nN\nE";
            text_draw(&text);
        } else {
            char buff[1024] = {0};
            uint pos = 0;

            for (int i = 0; i < n; i++) {
                pos += snprintf(buff + pos, sizeof(buff) - pos, "%s\n", results[i].name);
            }

            text.text = buff;
            text_draw(&text);
        }
        break;
    }

    default:
        debug("invalid main state %d", state);
        state_switch(STATE_SCAN);
        break;
    }
}
