/*
 * state.h
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

#ifndef __STATE_H__
#define __STATE_H__

enum system_state {
    STATE_INIT = 0,
    STATE_SCAN,
    STATE_VOLCANO_WORKFLOW,
    STATE_VOLCANO_RUN,
    STATE_CRAFTY,
    STATE_EDIT_WORKFLOW,

    STATE_INVALID,
};

void state_switch(enum system_state next);
void state_run(void);

#endif // __STATE_H__
