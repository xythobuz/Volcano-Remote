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
        state_scan_exit();
        break;

    default:
        break;
    }

    // prepare new state on entering
    switch (next) {
    case STATE_SCAN:
        debug("entering STATE_SCAN");
        state_scan_enter();
        break;

    default:
        break;
    }

    state = next;
}

void state_run(void) {
    switch (state) {
    case STATE_INIT:
        break;

    case STATE_SCAN: {
        state_scan_run();
        break;
    }

    default:
        debug("invalid main state %d", state);
        state_switch(STATE_SCAN);
        break;
    }
}
