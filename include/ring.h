/*
 * ring.h
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

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct ring_buffer {
    uint8_t *buffer;
    size_t size;
    size_t head, tail;
    bool full;
};
#define RB_INIT(b, s) { .buffer = b, .size = s, .head = 0, .tail = 0, .full = false }

void rb_add(struct ring_buffer *rb, const uint8_t *data, size_t length);
#define rb_push(rb, v) rb_add(rb, &v, 1)
size_t rb_len(struct ring_buffer *rb);
#define rb_space(rb) ((rb)->size - rb_len(rb))
void rb_dump(struct ring_buffer *rb, void (*write)(const uint8_t *, size_t));
void rb_move(struct ring_buffer *rb, void (*write)(const uint8_t *, size_t));
uint8_t rb_peek(struct ring_buffer *rb);
uint8_t rb_pop(struct ring_buffer *rb);

#endif // __RING_BUFFER_H__
