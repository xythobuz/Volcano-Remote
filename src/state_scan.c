/*
 * state_scan.c
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
#include "buttons.h"
#include "ble.h"
#include "lcd.h"
#include "text.h"
#include "models.h"
#include "state.h"

void state_scan_enter(void) {
    ble_scan(BLE_SCAN_ON);
}

void state_scan_exit(void) {
    ble_scan(BLE_SCAN_OFF);
}

void state_scan_run(void) {
    static char prev_buff[512] = {0};
    char buff[512] = {0};

    struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
    int n = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);

    uint pos = 0;
    for (int i = 0; i < n; i++) {
        enum known_devices dev = models_filter_name(results[i].name);
        if (dev == DEV_UNKNOWN) {
            continue;
        }

        if (dev == DEV_VOLCANO) {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "Volcano ");
        } else if (dev == DEV_CRAFTY) {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "Crafty+ ");
        }

        char info[32] = "";
        models_get_serial(results[i].data, results[i].data_len,
                          info, sizeof(info));
        pos += snprintf(buff + pos, sizeof(buff) - pos, "%s\n", info);
    }

    if (pos == 0) {
        strncpy(buff, "NONE", sizeof(buff));
    }

    if (strncmp(buff, prev_buff, sizeof(buff)) == 0) {
        return;
    }
    strncpy(prev_buff, buff, sizeof(prev_buff));

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

    text.text = buff;
    text_draw(&text);
}
