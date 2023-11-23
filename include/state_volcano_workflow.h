/*
 * state_volcano_workflow.h
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

#ifndef __STATE_VOLCANO_WORKFLOW_H__
#define __STATE_VOLCANO_WORKFLOW_H__

#include <stdbool.h>

void state_volcano_wf_edit(bool edit);

void state_volcano_wf_enter(void);
void state_volcano_wf_exit(void);
void state_volcano_wf_run(void);

#endif // __STATE_VOLCANO_WORKFLOW_H__
