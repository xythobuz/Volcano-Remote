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

static const int max_lines = 5;

static int menu_off = 0;
static int menu_selection = -1;
static int menu_length = 0;

static void text_box(const char *s) {
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

    // TODO clear background?!

    text.text = s;
    text_draw(&text);
}

static void state_scan_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_LEFT)) {
        // TODO brightness down
        return;
    } else if (state && (btn == BTN_RIGHT)) {
        // TODO brightness up
        return;
    } else if (state && ((btn == BTN_ENTER) || (btn == BTN_A))) {
        // TODO menu_selection
        return;
    } else if ((!state) || ((btn != BTN_UP) && (btn != BTN_DOWN))) {
        return;
    }

    if (state && (btn == BTN_UP)) {
        if (menu_selection < 0) {
            menu_selection = menu_length - 1;
        } else {
            menu_selection -= 1;
        }
    } else if (state && (btn == BTN_DOWN)) {
        if (menu_selection < 0) {
            menu_selection = 0;
        } else {
            menu_selection += 1;
        }
    }

    if (menu_selection < 0) {
        menu_selection += menu_length;
    }
    if (menu_selection >= menu_length) {
        menu_selection -= menu_length;
    }
    if (menu_selection >= 0) {
        while (menu_selection < menu_off) {
            menu_off -= 1;
        }
        while (menu_selection >= (menu_off + max_lines)) {
            menu_off += 1;
        }
    }
}

void state_scan_enter(void) {
    buttons_callback(state_scan_buttons);
    ble_scan(BLE_SCAN_ON);
}

void state_scan_exit(void) {
    buttons_callback(NULL);
    ble_scan(BLE_SCAN_OFF);
}

void state_scan_run(void) {
    static char prev_buff[512] = {0};
    char buff[512] = {0};

    struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
    int n = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);

    int pos = 0, devs = 0;
    for (int i = 0; i < n; i++) {
        enum known_devices dev = models_filter_name(results[i].name);
        if (dev == DEV_UNKNOWN) {
            continue;
        }

        devs++;

        if (((devs - 1) < menu_off)
            || ((devs - 1 - menu_off) >= max_lines)) {
            continue;
        }

#if defined(MENU_PREFER_VOLCANO) || defined(MENU_PREFER_CRAFTY)
        if (menu_selection < 0) {
#ifdef MENU_PREFER_VOLCANO
            if (dev == DEV_VOLCANO) {
                menu_selection = devs - 1;
            }
#endif // MENU_PREFER_VOLCANO
#ifdef MENU_PREFER_CRAFTY
            if (dev == DEV_CRAFTY) {
                menu_selection = devs - 1;
            }
#endif // MENU_PREFER_CRAFTY
        }
#endif // defined(MENU_PREFER_VOLCANO) || defined(MENU_PREFER_CRAFTY)

        if ((devs - 1) == menu_selection) {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "> ");
        } else {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "  ");
        }

        if (dev == DEV_VOLCANO) {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "Volcano ");
        } else if (dev == DEV_CRAFTY) {
            pos += snprintf(buff + pos, sizeof(buff) - pos, "Crafty+ ");
        }

        char info[32] = "";
        if (models_get_serial(results[i].data, results[i].data_len,
                              info, sizeof(info)) < 0) {
            strcpy(info, "-error-");
        }
        pos += snprintf(buff + pos, sizeof(buff) - pos, "%s\n", info);
    }

    menu_length = devs;

#if !defined(MENU_PREFER_VOLCANO) && !defined(MENU_PREFER_CRAFTY)
    if ((menu_selection < 0) && (menu_length > 0)) {
        menu_selection = 0;
    }
#endif // !defined(MENU_PREFER_VOLCANO) && !defined(MENU_PREFER_CRAFTY)

    if (menu_length == 0) {
        strncpy(buff, "NONE", sizeof(buff));
    }

    if (strncmp(buff, prev_buff, sizeof(buff)) != 0) {
        strncpy(prev_buff, buff, sizeof(prev_buff));
        text_box(buff);
    }
}
