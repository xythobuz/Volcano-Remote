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

enum wf_status {
    WF_IDLE = 0,
    WF_RUNNING,
};

uint16_t wf_count(void);
const char *wf_name(uint16_t index);

enum wf_status wf_status(void);
void wf_start(uint16_t index);

void wf_run(void);

#endif // __WORKFLOW_H__
