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

enum wf_status {
    WF_IDLE = 0,
    WF_RUNNING,
};

struct wf_state {
    enum wf_status status;

    uint16_t step;
    uint16_t count;
};

uint16_t wf_count(void);
const char *wf_name(uint16_t index);
const char *wf_author(uint16_t index);

struct wf_state wf_status(void);
void wf_start(uint16_t index);

void wf_reset(void);
void wf_run(void);

#endif // __WORKFLOW_H__
