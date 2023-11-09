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

#include "config.h"
#include "ring.h"

void rb_add(struct ring_buffer *rb, const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        rb->buffer[rb->head] = data[i];

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

void rb_dump(struct ring_buffer *rb, void (*write)(const uint8_t *, size_t)) {
    if (rb_len(rb) == 0) {
        return;
    }

    if (rb->head > rb->tail) {
        write(rb->buffer + rb->tail, rb->head - rb->tail);
    } else {
        write(rb->buffer + rb->tail, rb->size - rb->tail);
        write(rb->buffer, rb->head);
    }
}

void rb_move(struct ring_buffer *rb, void (*write)(const uint8_t *, size_t)) {
    rb_dump(rb, write);
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
}

uint8_t rb_peek(struct ring_buffer *rb) {
    if (rb_len(rb) == 0) {
        return 0;
    }

    uint8_t v = rb->buffer[rb->tail];
    return v;
}

uint8_t rb_pop(struct ring_buffer *rb) {
    if (rb_len(rb) == 0) {
        return 0;
    }

    uint8_t v = rb->buffer[rb->tail++];
    if (rb->tail >= rb->size) {
        rb->tail = 0;
    }

    return v;
}
