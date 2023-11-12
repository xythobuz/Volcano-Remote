/*
 * state_crafty.h
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

#ifndef __STATE_CRAFTY_H__
#define __STATE_CRAFTY_H__

#include <ble.h>

void state_crafty_target(bd_addr_t addr, bd_addr_type_t type);

void state_crafty_enter(void);
void state_crafty_exit(void);
void state_crafty_run(void);

#endif // __STATE_CRAFTY_H__
