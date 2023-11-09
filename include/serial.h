/*
 * serial.h
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

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void serial_init(void);
void serial_write(const uint8_t *buf, size_t count);
void serial_set_reroute(bool reroute);
void serial_run(void);

#endif // __SERIAL_H__
