/*
 * workflow.h
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

#ifndef __WORKFLOW_H__
#define __WORKFLOW_H__

#include <stdint.h>

#define WF_MAX_STR_LEN 10
#define WF_MAX_STEPS 42
#define WF_MAX_FLOWS 6

enum wf_op {
    OP_SET_TEMPERATURE = 0,
    OP_WAIT_TEMPERATURE,
    OP_WAIT_TIME,
    OP_PUMP_TIME,
};

struct wf_step {
    enum wf_op op;
    uint16_t val;
};

struct workflow {
    char name[WF_MAX_STR_LEN];
    char author[WF_MAX_STR_LEN];
    struct wf_step steps[WF_MAX_STEPS];
    uint16_t count;
};

enum wf_status {
    WF_IDLE = 0,
    WF_RUNNING,
};

struct wf_state {
    enum wf_status status;

    uint16_t index;
    uint16_t count;
    struct wf_step *step;
    uint16_t start_val, curr_val;
};

uint16_t wf_count(void);

void wf_move_down(uint16_t index);
void wf_move_up(uint16_t index);

const char *wf_name(uint16_t index);
const char *wf_author(uint16_t index);

uint16_t wf_steps(uint16_t index);

void wf_move_step_down(uint16_t index, uint16_t step);
void wf_move_step_up(uint16_t index, uint16_t step);

struct wf_step *wf_get_step(uint16_t index, uint16_t step);
const char *wf_step_str(struct wf_step *step);

struct wf_state wf_status(void);
void wf_start(uint16_t index);

void wf_reset(void);
void wf_run(void);

extern const uint16_t wf_default_count;
extern const struct workflow wf_default_data[];

#endif // __WORKFLOW_H__
