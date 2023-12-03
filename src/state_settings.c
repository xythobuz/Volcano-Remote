/*
 * state_settings.c
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
#include "log.h"
#include "menu.h"
#include "mem.h"
#include "state.h"
#include "state_value.h"
#include "state_workflow.h"
#include "state_settings.h"

static void enter_cb(int selection) {
    switch (selection) {
    case 0:
        // Auto Connect
        state_value_set(&mem_data()->wf_auto_connect,
                        sizeof(mem_data()->wf_auto_connect),
                        0, 1, VAL_STEP_INCREMENT, 1,
                        "Auto Connect");
        state_value_return(STATE_SETTINGS);
        state_switch(STATE_VALUE);
        break;

    case 1:
        // Brightness
        state_value_set(&mem_data()->backlight,
                        sizeof(mem_data()->backlight),
                        0x00FF, 0xFF00, VAL_STEP_SHIFT, 1,
                        "Brightness");
        state_value_return(STATE_SETTINGS);
        state_switch(STATE_VALUE);
        break;

    case 2:
        // Workflows
        state_wf_edit(true);
        state_switch(STATE_WORKFLOW);
        break;

    case 3:
        // Reset
        mem_load_defaults();
        break;
    }
}

static void exit_cb(void) {
    state_switch(STATE_SCAN);
}

void state_settings_enter(void) {
    menu_init(enter_cb, NULL, NULL, exit_cb);
}

void state_settings_exit(void) {
    menu_deinit();
    mem_write();
}

static void draw(struct menu_state *menu) {
    int pos = 0;
    menu->length = 0;

    ADD_STATIC_ELEMENT("Auto Connect (%d)", mem_data()->wf_auto_connect);
    ADD_STATIC_ELEMENT("Brightness (%d)", __builtin_ffs(mem_data()->backlight));
    ADD_STATIC_ELEMENT("Edit Workflows");
    ADD_STATIC_ELEMENT("Factory Reset");

    if (menu->selection < 0) {
        menu->selection = 0;
    }
}

void state_settings_run(void) {
    menu_run(draw, false);
}
