/*
 * state_value.c
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
#include "buttons.h"
#include "log.h"
#include "lcd.h"
#include "text.h"
#include "state_value.h"

static void *val_p = NULL;
static size_t val_len = 0;
static ssize_t val_min = 0;
static ssize_t val_max = 0;
static ssize_t val_step = 0;
static const char *val_name = NULL;

static enum value_step_mode val_mode = VAL_STEP_INCREMENT;
static enum system_state val_ret_state = STATE_SCAN;

static ssize_t val = 0;

void state_value_set(void *value, size_t length,
                     ssize_t min, ssize_t max,
                     enum value_step_mode mode, ssize_t step,
                     const char *name) {
    val_p = value;
    val_len = length;
    val_min = min;
    val_max = max;
    val_mode = mode;
    val_step = step;
    val_name = name;
}

void state_value_return(enum system_state state) {
    val_ret_state = state;
}

static void draw(void) {
    static char buff[100];
    static size_t pos = 0;

    if ((val_p == NULL) || (val_len <= 0) || (val_name == NULL)) {
        pos += snprintf(buff, sizeof(buff),
                        "error");
    } else {
        if (val_mode == VAL_STEP_INCREMENT) {
            pos += snprintf(buff, sizeof(buff),
                            "%s:\n%d -> %d -> %d",
                            val_name, val_min / val_step, val / val_step, val_max / val_step);
        } else {
            pos += snprintf(buff, sizeof(buff),
                            "%s:\n%04X -> %04X -> %04X",
                            val_name, val_min, val, val_max);
        }
    }

    text_box(buff, true,
             "fixed_10x20",
             0, LCD_WIDTH,
             50, TEXT_BOX_HEIGHT(20, 2),
             0);
}

static void write(void) {
    if ((val_p == NULL) || (val_len <= 0)) {
        debug("invalid params");
        return;
    }

    switch (val_len) {
    case 1:
        *((uint8_t *)val_p) = val;
        break;

    case 2:
        *((uint16_t *)val_p) = val;
        break;

    case 4:
        *((uint32_t *)val_p) = val;
        break;

    default:
        debug("invalid len %d", val_len);
        return;
    }
}

static void step(ssize_t v) {
    if ((val_p == NULL) || (val_len <= 0)) {
        debug("invalid params");
        return;
    }

    if (((v > 0) && (val >= val_max))
        || ((v < 0) && (val <= val_min))) {
        return;
    }

    switch (val_mode) {
    case VAL_STEP_INCREMENT:
        val += v * val_step;
        break;

    case VAL_STEP_SHIFT:
        if ((val_step * v) > 0) {
            val <<= val_step;
        } else {
            val >>= val_step;
        }
        break;
    }

    // apply new value while editing
    write();
}

static void value_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        state_switch(val_ret_state);
    } else if (state && (btn == BTN_LEFT)) {
        step(-1);
        draw();
    } else if (state && (btn == BTN_RIGHT)) {
        step(1);
        draw();
    }
}

void state_value_enter(void) {
    buttons_callback(value_buttons);

    switch (val_len) {
    case 1:
        val = *((uint8_t *)val_p);
        break;

    case 2:
        val = *((uint16_t *)val_p);
        break;

    case 4:
        val = *((uint32_t *)val_p);
        break;

    default:
        debug("invalid len %d", val_len);
        return;
    }

    draw();
}

void state_value_exit(void) {
    buttons_callback(NULL);
    write();
}

void state_value_run(void) { }
