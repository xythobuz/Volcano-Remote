/*
 * state.c
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

#include "config.h"
#include "log.h"
#include "state_scan.h"
#include "state_workflow.h"
#include "state_volcano_run.h"
#include "state_crafty.h"
#include "state_edit_workflow.h"
#include "state_settings.h"
#include "state_about.h"
#include "state_value.h"
#include "state.h"

#define stringify(name) # name

struct state {
    const char * const name;
    void (*enter)(void);
    void (*exit)(void);
    void (*run)(void);
};

static const struct state states[STATE_INVALID + 1] = {
    {
        .name = stringify(STATE_INIT),
        .enter = NULL,
        .exit = NULL,
        .run = NULL,
    }, {
        .name = stringify(STATE_SCAN),
        .enter = state_scan_enter,
        .exit = state_scan_exit,
        .run = state_scan_run,
    }, {
        .name = stringify(STATE_WORKFLOW),
        .enter = state_wf_enter,
        .exit = state_wf_exit,
        .run = state_wf_run,
    }, {
        .name = stringify(STATE_VOLCANO_RUN),
        .enter = state_volcano_run_enter,
        .exit = state_volcano_run_exit,
        .run = state_volcano_run_run,
    }, {
        .name = stringify(STATE_CRAFTY),
        .enter = state_crafty_enter,
        .exit = state_crafty_exit,
        .run = state_crafty_run,
    }, {
        .name = stringify(STATE_EDIT_WORKFLOW),
        .enter = state_edit_wf_enter,
        .exit = state_edit_wf_exit,
        .run = state_edit_wf_run,
    }, {
        .name = stringify(STATE_SETTINGS),
        .enter = state_settings_enter,
        .exit = state_settings_exit,
        .run = state_settings_run,
    }, {
        .name = stringify(STATE_ABOUT),
        .enter = state_about_enter,
        .exit = state_about_exit,
        .run = state_about_run,
    }, {
        .name = stringify(STATE_VALUE),
        .enter = state_value_enter,
        .exit = state_value_exit,
        .run = state_value_run,
    }, {
        .name = stringify(STATE_INVALID),
        .enter = NULL,
        .exit = NULL,
        .run = NULL,
    }
};

static enum system_state state = STATE_INIT;

void state_switch(enum system_state next) {
    if (state == next) {
        return;
    }

    if (next > STATE_INVALID) {
        debug("invalid new state %d", next);
        next = STATE_INVALID;
    }

    debug("leaving %s", states[state].name);
    if (states[state].exit) {
        states[state].exit();
    }

    debug("entering %s", states[next].name);
    if (states[next].enter) {
        states[next].enter();
    }

    state = next;
}

void state_run(void) {
    if (state >= STATE_INVALID) {
        debug("invalid main state %d", state);
        state_switch(STATE_SCAN);
    }

    if (states[state].run) {
        states[state].run();
    }
}
