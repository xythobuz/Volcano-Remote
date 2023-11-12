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

#include "config.h"
#include "log.h"
#include "volcano.h"
#include "workflow.h"

#define WF_MAX_STEPS 42
#define WF_MAX_FLOWS 6

struct workflow {
    const char *name;
    const char *author;
    struct wf_step steps[WF_MAX_STEPS];
    uint16_t count;
};

#define NOTIFY \
    { .op = OP_WAIT_TIME, .val = 1000 }, \
    { .op = OP_PUMP_TIME, .val = 1000 }

static const struct workflow wf[WF_MAX_FLOWS] = {
    {
        .name = "Default",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1850 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1950 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2050 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    }, {
        .name = "Vorbi",
        .author = "Rinor",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1760 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 6000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1870 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 10000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2040 },
            { .op = OP_WAIT_TIME, .val = 3000 },
            { .op = OP_PUMP_TIME, .val = 10000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2170 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 10000 },
        },
        .count = 12,
    }, {
        .name = "Relaxo",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1750 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1850 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1950 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    }, {
        .name = "Hotty",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1900 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2050 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2200 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    },
};

static const uint16_t count = 4;

static enum wf_status status = WF_IDLE;
static uint16_t wf_i = 0;
static uint16_t step = 0;
static uint32_t start_t = 0;
static uint16_t start_val = 0;
static uint16_t curr_val = 0;

static void do_step(void) {
    switch (wf[wf_i].steps[step].op) {
    case OP_SET_TEMPERATURE:
    case OP_WAIT_TEMPERATURE:
        debug("workflow temp %.1f C", wf[wf_i].steps[step].val / 10.0);
        start_val = volcano_get_current_temp();
        volcano_set_target_temp(wf[wf_i].steps[step].val);
        break;

    case OP_PUMP_TIME:
        volcano_set_pump_state(true);
        start_t = to_ms_since_boot(get_absolute_time());
        start_val = 0;
        debug("workflow pump %.3f s", wf[wf_i].steps[step].val / 1000.0);
        break;

    case OP_WAIT_TIME:
        start_t = to_ms_since_boot(get_absolute_time());
        start_val = 0;
        debug("workflow time %.3f s", wf[wf_i].steps[step].val / 1000.0);
        break;
    }

    curr_val = start_val;
}

uint16_t wf_count(void) {
    return count;
}

const char *wf_name(uint16_t index) {
    if (index >= count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return wf[index].name;
}

const char *wf_author(uint16_t index) {
    if (index >= count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return wf[index].author;
}

struct wf_state wf_status(void) {
    struct wf_state s = {
        .status = status,
        .index = step,
        .count = wf[wf_i].count,
        .step = wf[wf_i].steps[step],
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
    if (index >= count) {
        debug("invalid index %d", index);
        return;
    }

    status = WF_RUNNING;
    wf_i = index;
    step = 0;

    // discover characteristics
    volcano_set_pump_state(false);
    volcano_set_heater_state(false);
    volcano_set_target_temp(1850);

    volcano_set_heater_state(true);

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

    switch (wf[wf_i].steps[step].op) {
    case OP_SET_TEMPERATURE:
        done = true;
        break;

    case OP_WAIT_TEMPERATURE: {
        uint16_t temp = volcano_get_current_temp();
        curr_val = temp;
        done = (temp >= (wf[wf_i].steps[step].val - 5));
        break;
    }

    case OP_PUMP_TIME:
    case OP_WAIT_TIME: {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint32_t diff = now - start_t;
        curr_val = diff;
        done = (diff >= wf[wf_i].steps[step].val);
        break;
    }
    }

    if (done) {
        if (wf[wf_i].steps[step].op == OP_PUMP_TIME) {
            volcano_set_pump_state(false);
        }

        step++;
        if (step >= wf[wf_i].count) {
            status = WF_IDLE;
            volcano_set_heater_state(false);
            debug("workflow finished");
        } else {
            do_step();
        }
    }
}
