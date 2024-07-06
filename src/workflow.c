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

#if defined(INFLUXDB_HOST) && defined(INFLUXDB_PORT) && defined(INFLUXDB_DATABASE)
#include "usb_descriptors.h"
#include "wifi.h"
#define VOLCANO_INFLUX_DB
#endif

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

#ifdef VOLCANO_INFLUX_DB
static void influxdb_send(const char *name, double value) {
    if (!wifi_ready()) {
        debug("skip influx, no wifi (\"%s\" %.2lf)", name, value);
        return;
    }

    char payload[128] = {0};
    size_t len = 0;
    len = snprintf(payload, sizeof(payload) - 1,
                   "volcano,device=%s %s=%lf",
                   string_pico_serial, name, value);


    char packet[256] = {0};
    size_t n = 0;
    n = snprintf(packet, sizeof(packet) - 1,
                  "POST /write?db=%s HTTP/1.0\r\n"
                  "Host: %s\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n"
                  "%s",
                  INFLUXDB_DATABASE, INFLUXDB_HOST,
                 len, payload);

    debug("write influx \"%s\": %.2lf", name, value);
    wifi_tcp_send(INFLUXDB_HOST, INFLUXDB_PORT, packet, n);
}
#endif // VOLCANO_INFLUX_DB

static void do_step(void) {
    switch (mem_data()->wf[wf_i].steps[step].op) {
    case OP_SET_TEMPERATURE:
    case OP_WAIT_TEMPERATURE:
        debug("workflow temp %.1f C", mem_data()->wf[wf_i].steps[step].val / 10.0);
        start_val = volcano_get_current_temp();
        DO_WHILE(volcano_set_target_temp(mem_data()->wf[wf_i].steps[step].val),
                 volcano_get_target_temp() != mem_data()->wf[wf_i].steps[step].val);
#ifdef VOLCANO_INFLUX_DB
        influxdb_send("target", mem_data()->wf[wf_i].steps[step].val);
#endif // VOLCANO_INFLUX_DB
        break;

    case OP_PUMP_TIME:
        DO_WHILE(volcano_set_pump_state(true),
                 !(volcano_get_state() & VOLCANO_STATE_PUMP));
        start_t = to_ms_since_boot(get_absolute_time());
        start_val = 0;
        debug("workflow pump %.3f s", mem_data()->wf[wf_i].steps[step].val / 1000.0);
#ifdef VOLCANO_INFLUX_DB
        influxdb_send("pump", 1);
#endif // VOLCANO_INFLUX_DB
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

void wf_move_step_down(uint16_t index, uint16_t step_i) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return;
    }
    if ((step_i < 1) || (step_i >= mem_data()->wf[index].count)) {
        debug("invalid step %d", step_i);
        return;
    }

    struct wf_step tmp = mem_data()->wf[index].steps[step_i - 1];
    mem_data()->wf[index].steps[step_i - 1] = mem_data()->wf[index].steps[step_i];
    mem_data()->wf[index].steps[step_i] = tmp;
}

void wf_move_step_up(uint16_t index, uint16_t step_i) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return;
    }
    if (step_i >= (mem_data()->wf[index].count - 1)) {
        debug("invalid step %d", step_i);
        return;
    }

    struct wf_step tmp = mem_data()->wf[index].steps[step_i + 1];
    mem_data()->wf[index].steps[step_i + 1] = mem_data()->wf[index].steps[step_i];
    mem_data()->wf[index].steps[step_i] = tmp;
}

struct wf_step *wf_get_step(uint16_t index, uint16_t step_i) {
    if (index >= mem_data()->wf_count) {
        debug("invalid index %d", index);
        return NULL;
    }
    return &mem_data()->wf[index].steps[step_i];
}

const char *wf_step_str(struct wf_step *step_p) {
    static char buff[20];

    switch (step_p->op) {
    case OP_SET_TEMPERATURE:
        snprintf(buff, sizeof(buff),
                 "set temp %.1f C", step_p->val / 10.0f);
        break;

    case OP_WAIT_TEMPERATURE:
        snprintf(buff, sizeof(buff),
                 "wait temp %.1f C", step_p->val / 10.0f);
        break;

    case OP_WAIT_TIME:
    case OP_PUMP_TIME:
        snprintf(buff, sizeof(buff),
                 "%s time %.1f s",
                 (step_p->op == OP_WAIT_TIME) ? "wait" : "pump",
                 step_p->val / 1000.0f);
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

#ifdef VOLCANO_INFLUX_DB
    influxdb_send("heater", 1);
#endif // VOLCANO_INFLUX_DB

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

#ifdef VOLCANO_INFLUX_DB
        static uint16_t last_temp = 0;
        if (last_temp != temp) {
            influxdb_send("current", temp);
            last_temp = temp;
        }
#endif // VOLCANO_INFLUX_DB

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
#ifdef VOLCANO_INFLUX_DB
            influxdb_send("pump", 0);
#endif // VOLCANO_INFLUX_DB
        }

        step++;
        if (step >= mem_data()->wf[wf_i].count) {
            status = WF_IDLE;
            DO_WHILE(volcano_set_heater_state(false),
                     volcano_get_state() & VOLCANO_STATE_HEATER);
            debug("workflow finished");

#ifdef VOLCANO_INFLUX_DB
            influxdb_send("heater", 0);
#endif // VOLCANO_INFLUX_DB
        } else {
            do_step();
        }
    }
}
