/*
 * state_wifi_edit.h
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

#ifndef __STATE_WIFI_EDIT_H__
#define __STATE_WIFI_EDIT_H__

#include <stdint.h>

void state_wifi_edit_index(uint16_t index);

void state_wifi_edit_enter(void);
void state_wifi_edit_exit(void);
void state_wifi_edit_run(void);

#endif // __STATE_WIFI_EDIT_H__
