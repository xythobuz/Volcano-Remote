/*
 * cache.h
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

#ifndef __CACHE_H__
#define __CACHE_H__

#include <sys/types.h>

void cache_init(void);
void cache_status(void);
void cache_sync(void);
void cache_run(void);

ssize_t cache_read(uint8_t *buf, size_t addr, size_t len);
ssize_t cache_write(const uint8_t *buf, size_t addr, size_t len);

#endif // __CACHE_H__
