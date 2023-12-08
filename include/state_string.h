/*
 * state_string.h
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

#ifndef __STATE_STRING_H__
#define __STATE_STRING_H__

#include <sys/types.h>
#include "state.h"

void state_string_set(char *value, size_t length,
                      const char *name);
void state_string_return(enum system_state state);

void state_string_enter(void);
void state_string_exit(void);
void state_string_run(void);

#endif // __STATE_STRING_H__
