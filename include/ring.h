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
    void *buffer;
    size_t size;
    size_t el_len;
    size_t head, tail;
    bool full;
};
#define RB_INIT(b, s, e) { .buffer = b, .size = s, .el_len = e, .head = 0, .tail = 0, .full = false }

void rb_add(struct ring_buffer *rb, const void *data, size_t length);
#define rb_push(rb, v) rb_add(rb, v, 1)
size_t rb_len(struct ring_buffer *rb);
#define rb_space(rb) ((rb)->size - rb_len(rb))
void rb_dump(struct ring_buffer *rb, void (*write)(const void *, size_t), size_t skip);
void rb_move(struct ring_buffer *rb, void (*write)(const void *, size_t));
void rb_peek(struct ring_buffer *rb, void *buf);
void rb_pop(struct ring_buffer *rb, void *buf);
size_t rb_get(struct ring_buffer *rb, void *data, size_t length);

#endif // __RING_BUFFER_H__
