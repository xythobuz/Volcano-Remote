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
#include <string.h>

#include "pico/stdlib.h"
#include "picowota/reboot.h"

#include "config.h"
#include "ble.h"
#include "models.h"
#include "mem.h"
#include "menu.h"
#include "state.h"
#include "state_workflow.h"
#include "state_volcano_run.h"
#include "state_volcano_conf.h"
#include "state_crafty.h"
#include "state_venty.h"
#include "state_scan.h"

static struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
static int result_count = 0;
static uint32_t auto_connect_time = 0;
static int auto_connect_idx = 0;

static void enter_cb(int selection) {
    int devs = 0;
    for (int i = 0; i < result_count; i++) {
        enum known_devices dev = models_filter_name(results[i].name);
        if (dev == DEV_UNKNOWN) {
            continue;
        }

        if (devs++ == selection) {
            if (dev == DEV_VOLCANO) {
                state_volcano_run_target(results[i].addr, results[i].type);
                state_wf_edit(false);
                state_switch(STATE_WORKFLOW);
            } else if (dev == DEV_CRAFTY) {
                state_crafty_target(results[i].addr, results[i].type);
                state_switch(STATE_CRAFTY);
            } else if (dev == DEV_VENTY) {
                state_venty_target(results[i].addr, results[i].type);
                state_switch(STATE_VENTY);
            }
            return;
        }
    }

    if (selection == devs) {
        state_switch(STATE_SETTINGS);
    } else if (selection == (devs + 1)) {
        state_switch(STATE_ABOUT);
    }
}

static void edit_cb(int selection) {
    int devs = 0;
    for (int i = 0; i < result_count; i++) {
        enum known_devices dev = models_filter_name(results[i].name);
        if (dev == DEV_UNKNOWN) {
            continue;
        }

        if (devs++ == selection) {
            if (dev == DEV_VOLCANO) {
                state_volcano_conf_target(results[i].addr, results[i].type);
                state_switch(STATE_VOLCANO_CONF);
            } else if (dev == DEV_CRAFTY) {
                state_crafty_target(results[i].addr, results[i].type);
                state_switch(STATE_CRAFTY);
            } else if (dev == DEV_VENTY) {
                state_venty_target(results[i].addr, results[i].type);
                state_switch(STATE_VENTY);
            }
            return;
        }
    }

    if (selection == devs) {
        state_switch(STATE_SETTINGS);
    } else if (selection == (devs + 1)) {
        state_switch(STATE_ABOUT);
    }
}

static void ota_cb(int selection) {
    UNUSED(selection);
    picowota_reboot(true);
}

void state_scan_enter(void) {
    menu_init(enter_cb, edit_cb, ota_cb, NULL);
    ble_scan(BLE_SCAN_ON);
}

void state_scan_exit(void) {
    ble_scan(BLE_SCAN_OFF);
    menu_deinit();
}

static void draw(struct menu_state *menu) {
    result_count = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);

    int pos = 0, devs = 0;
    for (int i = 0; i < result_count; i++) {
        enum known_devices dev = models_filter_name(results[i].name);
        if (dev == DEV_UNKNOWN) {
            continue;
        }

        devs++;

        if (((devs - 1) < menu->off)
            || ((devs - 1 - menu->off) >= MENU_MAX_LINES)) {
            continue;
        }

#if defined(MENU_PREFER_VOLCANO) || defined(MENU_PREFER_CRAFTY)
        if (menu->selection < 0) {
#ifdef MENU_PREFER_VOLCANO
            if (dev == DEV_VOLCANO) {
                menu->selection = devs - 1;

                if (to_ms_since_boot(get_absolute_time()) <= VOLCANO_AUTO_CONNECT_WITHIN_MS) {
                    if (mem_data()->wf_auto_connect) {
                        auto_connect_time = to_ms_since_boot(get_absolute_time());
                        auto_connect_idx = i;
                    }
                }
            }
#endif // MENU_PREFER_VOLCANO
#ifdef MENU_PREFER_CRAFTY
            if (dev == DEV_CRAFTY) {
                menu->selection = devs - 1;
            }
#endif // MENU_PREFER_CRAFTY
        }
#endif // defined(MENU_PREFER_VOLCANO) || defined(MENU_PREFER_CRAFTY)

        if ((devs - 1) == menu->selection) {
            if ((auto_connect_time != 0) && (!menu_got_input)) {
                uint32_t now = to_ms_since_boot(get_absolute_time());
                uint32_t diff = now - auto_connect_time;
                pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos,
                                "%ld ",
                                (VOLCANO_AUTO_CONNECT_TIMEOUT_MS / 1000) - (diff / 1000));
            } else {
                pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "> ");
            }
        } else {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "  ");
        }

        if (dev == DEV_VOLCANO) {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "Volcano ");
        } else if (dev == DEV_CRAFTY) {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "Crafty+ ");
        } else if (dev == DEV_VENTY) {
            pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "Venty ");
        }

        char info[32] = "";
        if (models_get_serial(dev, results[i].name,
                              results[i].data, results[i].data_len,
                              info, sizeof(info)) < 0) {
            strcpy(info, "-error-");
        }
        pos += snprintf(menu->buff + pos, MENU_MAX_LEN - pos, "%s\n", info);
    }

    menu->length = devs;

#if !defined(MENU_PREFER_VOLCANO) && !defined(MENU_PREFER_CRAFTY)
    if ((menu->selection < 0) && (menu->length > 0)) {
        menu->selection = 0;
    }
#endif // !defined(MENU_PREFER_VOLCANO) && !defined(MENU_PREFER_CRAFTY)

    ADD_STATIC_ELEMENT("Settings");
    ADD_STATIC_ELEMENT("About");
}

void state_scan_run(void) {
    menu_run(draw, false);

    if ((auto_connect_time != 0) && (!menu_got_input)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - auto_connect_time) >= VOLCANO_AUTO_CONNECT_TIMEOUT_MS) {
            state_volcano_run_target(results[auto_connect_idx].addr,
                                     results[auto_connect_idx].type);
            state_wf_edit(false);
            state_switch(STATE_WORKFLOW);

            auto_connect_time = 0;
        }
    }
}
