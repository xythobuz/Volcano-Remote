/*
 * ring.c
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

#define MIN(x, y) ((x < y) ? x : y)

#include <string.h>

#include "config.h"
#include "ring.h"

void rb_add(struct ring_buffer *rb, const void *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        memcpy(rb->buffer + rb->head * rb->el_len, data + i * rb->el_len, rb->el_len);

        if (rb->full && (++(rb->tail) == rb->size)) {
            rb->tail = 0;
        }

        if (++(rb->head) == rb->size) {
            rb->head = 0;
        }

        rb->full = ((rb->head) == (rb->tail));
    }
}

size_t rb_len(struct ring_buffer *rb) {
    if (rb->head == rb->tail) {
        if (rb->full) {
            return rb->size;
        } else {
            return 0;
        }
    } else if (rb->head > rb->tail) {
        return rb->head - rb->tail;
    } else {
        return rb->size - rb->tail + rb->head;
    }
}

void rb_dump(struct ring_buffer *rb, void (*write)(const void *, size_t), size_t skip) {
    if (rb_len(rb) <= skip) {
        return;
    }

    if (rb->head > rb->tail) {
        if ((rb->head - rb->tail) > skip) {
            write(rb->buffer + ((rb->tail + skip) * rb->el_len), rb->head - rb->tail - skip);
        }
    } else {
        if ((rb->size - rb->tail) > skip) {
            write(rb->buffer + ((rb->tail + skip) * rb->el_len), rb->size - rb->tail - skip);
        }

        skip -= MIN(skip, rb->size - rb->tail);
        if (rb->head > skip) {
            write(rb->buffer + (skip + rb->el_len), rb->head - skip);
        }
    }
}

void rb_move(struct ring_buffer *rb, void (*write)(const void *, size_t)) {
    rb_dump(rb, write, 0);
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
}

void rb_peek(struct ring_buffer *rb, void *buf) {
    if (rb_len(rb) == 0) {
        return;
    }

    memcpy(buf, rb->buffer + rb->tail * rb->el_len, rb->el_len);
}

void rb_pop(struct ring_buffer *rb, void *buf) {
    if (rb_len(rb) == 0) {
        return;
    }

    memcpy(buf, rb->buffer + rb->tail * rb->el_len, rb->el_len);
    rb->tail++;
    if (rb->tail >= rb->size) {
        rb->tail = 0;
    }
}

size_t rb_get(struct ring_buffer *rb, void *data, size_t length) {
    size_t count = 0;
    while ((length > 0) && (rb_len(rb) > 0)) {
        rb_pop(rb, data + count * rb->el_len);
        count++;
        length--;
    }
    return count;
}
