/*
 * state_value.h
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

#ifndef __STATE_VALUE_H__
#define __STATE_VALUE_H__

#include <sys/types.h>
#include "state.h"

enum value_step_mode {
    VAL_STEP_INCREMENT = 0,
    VAL_STEP_SHIFT,
};

void state_value_set(void *value, size_t length,
                     ssize_t min, ssize_t max,
                     enum value_step_mode mode, ssize_t step);
void state_value_return(enum system_state state);

void state_value_enter(void);
void state_value_exit(void);
void state_value_run(void);

#endif // __STATE_VALUE_H__
