/*
 * mem.h
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

#ifndef __MEM_H__
#define __MEM_H__

#include <stdint.h>

#define MEM_VERSION 0x01

struct mem_data {
    uint8_t version;
    uint16_t backlight;
} __attribute__((packed));

#define MEM_DATA_INIT {         \
    .version = MEM_VERSION,     \
    .backlight = (0xFF00 >> 1), \
}

void mem_init(void);
void mem_write(void);
struct mem_data *mem_data(void);

#endif // __MEM_H__
