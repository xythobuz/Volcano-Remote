/*
 * workflow.c
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

#define WF_CONFIRM_WRITES

#include <stdio.h>

#include "config.h"
#include "log.h"
#include "mem.h"
#include "volcano.h"
#include "workflow.h"

#ifdef WF_CONFIRM_WRITES
#define DO_WHILE(x, y) \
    do {               \
        x;             \
    } while (y)
#else // WF_CONFIRM_WRITES
#define DO_WHILE(x, y) x
#endif // WF_CONFIRM_WRITES

static enum wf_status status = WF_IDLE;
static uint16_t wf_i = 0;
static uint16_t step = 0;
static uint32_t start_t = 0;
static uint16_t start_val = 0;
static uint16_t curr_val = 0;

static void do_step(void) {
    switch (mem_data()->wf[wf_i].steps[step].op) {
    case OP_SET_TEMPERATURE:
    case OP_WAIT_TEMPERATURE:
        debug("workflow temp %.1f C", mem_data()->wf[wf_i].steps[step].val / 10.0);
        start_val = volcano_get_current_temp();
        DO_WHILE(volcano_set_target_temp(mem_data()->wf[wf_i].steps[step].val),
                 volcano_get_target_temp() != mem_data()->wf[wf_i].steps[step].val);
        break;

    case OP_PUMP_TIME:
        DO_WHILE(volcano_set_pump_state(true),
                 !(volcano_get_state() & VOLCANO_STATE_PUMP));
        start_t = to_ms_since_boot(get_absolute_time());
        start_val = 0;
        debug("workflow pump %.3f s", mem_data()->wf[wf_i].steps[step].val / 1000.0);
        break;

    case OP_WAIT_TIME:
        start_t = to_ms_since_boot(get_absolute_time());
        start_val = 0;
        debug("workflow time %.3f s", mem_data()->wf[wf_i].steps[step].val / 1000.0);
        break;
    }

    curr_val = start_val;
}

uint16_t wf_count(void) {
    return mem_data()->wf_count;
}

void wf_move_down(uint16_t index) {
    if ((index < 1) || (index >= mem_data()->wf_count)) {
        debug("invalid index %d", index);
        return;
    }

    struct workflow tmp = mem_data()->wf[index - 1];
    mem_data()->wf[index - 1] = mem_data()->wf[index];
    mem_data()->wf[index] = tmp;
}

void wf_move_up(uint16_t index) {
    if (index >= (mem_data()->wf_count - 1)) {
        debug("invalid index %d", index);
        return;
    }

    struct workflow tmp = mem_data()->wf[index + 1];
    mem_data()->wf[index + 1] = mem_data()->wf[index];
    mem_data()->wf[index] = tmp;
}

uint16_t wf_steps(uint16_t index) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return 0;
    }
    return mem_data()->wf[index].count;
}

void wf_move_step_down(uint16_t index, uint16_t step) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return;
    }
    if ((step < 1) || (step >= mem_data()->wf[index].count)) {
        debug("invalid step %d", step);
        return;
    }

    struct wf_step tmp = mem_data()->wf[index].steps[step - 1];
    mem_data()->wf[index].steps[step - 1] = mem_data()->wf[index].steps[step];
    mem_data()->wf[index].steps[step] = tmp;
}

void wf_move_step_up(uint16_t index, uint16_t step) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return;
    }
    if (step >= (mem_data()->wf[index].count - 1)) {
        debug("invalid step %d", step);
        return;
    }

    struct wf_step tmp = mem_data()->wf[index].steps[step + 1];
    mem_data()->wf[index].steps[step + 1] = mem_data()->wf[index].steps[step];
    mem_data()->wf[index].steps[step] = tmp;
}

struct wf_step *wf_get_step(uint16_t index, uint16_t step) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return &mem_data()->wf[index].steps[step];
}

const char *wf_step_str(struct wf_step *step) {
    static char buff[20];

    switch (step->op) {
    case OP_SET_TEMPERATURE:
        snprintf(buff, sizeof(buff),
                 "set temp %.1f C", step->val / 10.0f);
        break;

    case OP_WAIT_TEMPERATURE:
        snprintf(buff, sizeof(buff),
                 "wait temp %.1f C", step->val / 10.0f);
        break;

    case OP_WAIT_TIME:
    case OP_PUMP_TIME:
        snprintf(buff, sizeof(buff),
                 "%s time %.1f s",
                 (step->op == OP_WAIT_TIME) ? "wait" : "pump",
                 step->val / 1000.0f);
        break;
    }

    return buff;
}

const char *wf_name(uint16_t index) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return mem_data()->wf[index].name;
}

const char *wf_author(uint16_t index) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return mem_data()->wf[index].author;
}

struct wf_state wf_status(void) {
    struct wf_state s = {
        .status = status,
        .index = step,
        .count = mem_data()->wf[wf_i].count,
        .step = &mem_data()->wf[wf_i].steps[step],
        .start_val = start_val,
        .curr_val = curr_val,
    };
    return s;
}

void wf_start(uint16_t index) {
    if (status != WF_IDLE) {
        debug("workflow already running");
        return;
    }
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return;
    }

    status = WF_RUNNING;
    wf_i = index;
    step = 0;

    /*
     * first turn on heater, then do discovery, to save some time.
     * this means we heat for some seconds before changing the setpoint.
     * should not be a problem in practice.
     */
    DO_WHILE(volcano_set_heater_state(true),
             !(volcano_get_state() & VOLCANO_STATE_HEATER));
    volcano_discover_characteristics(true, false);

    do_step();
}

void wf_reset(void) {
    status = WF_IDLE;
}

void wf_run(void) {
    if (status == WF_IDLE) {
        return;
    }

    bool done = false;

    switch (mem_data()->wf[wf_i].steps[step].op) {
    case OP_SET_TEMPERATURE:
        done = true;
        break;

    case OP_WAIT_TEMPERATURE: {
        uint16_t temp = volcano_get_current_temp();

        // volcano does not provide a temperature when cold
        if (start_val == 0) {
            start_val = temp;
        }

        curr_val = temp;
        done = (temp >= (mem_data()->wf[wf_i].steps[step].val - 5));
        break;
    }

    case OP_PUMP_TIME:
    case OP_WAIT_TIME: {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint32_t diff = now - start_t;
        curr_val = diff;
        done = (diff >= mem_data()->wf[wf_i].steps[step].val);
        break;
    }
    }

    if (done) {
        if (mem_data()->wf[wf_i].steps[step].op == OP_PUMP_TIME) {
            DO_WHILE(volcano_set_pump_state(false),
                     volcano_get_state() & VOLCANO_STATE_PUMP);
        }

        step++;
        if (step >= mem_data()->wf[wf_i].count) {
            status = WF_IDLE;
            DO_WHILE(volcano_set_heater_state(false),
                     volcano_get_state() & VOLCANO_STATE_HEATER);
            debug("workflow finished");
        } else {
            do_step();
        }
    }
}
