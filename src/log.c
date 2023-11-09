/*
 * log.c
 *
 * Copyright (c) 2022 - 2023 Thomas Buck (thomas@xythobuz.de)
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

#include <stdio.h>

#include "hardware/watchdog.h"
#include "ff.h"

#include "config.h"
#include "main.h"
#include "usb_cdc.h"
#include "serial.h"
#include "ring.h"
#include "log.h"

static uint8_t log_buff[4096] = {0};
static struct ring_buffer log = RB_INIT(log_buff, sizeof(log_buff));

static uint8_t line_buff[128] = {0};
static volatile bool got_input = false;
static FIL log_file_fat;

static void add_to_log(const uint8_t *buff, size_t len) {
    rb_add(&log, buff, len);
}

static void log_dump_to_x(void (*write)(const uint8_t *, size_t)) {
    if (rb_len(&log) == 0) {
        return;
    }

    int l = snprintf((char *)line_buff, sizeof(line_buff), "\r\n\r\nbuffered log output:\r\n");
    if ((l > 0) && (l <= (int)sizeof(line_buff))) {
        write(line_buff, l);
    }

    rb_dump(&log, write);

    l = snprintf((char *)line_buff, sizeof(line_buff), "\r\n\r\nlive log:\r\n");
    if ((l > 0) && (l <= (int)sizeof(line_buff))) {
        write(line_buff, l);
    }
}

void log_dump_to_usb(void) {
    log_dump_to_x(usb_cdc_write);
}

void log_dump_to_uart(void) {
    log_dump_to_x(serial_write);
}

static void log_file_write_callback(const uint8_t *data, size_t len) {
    UINT bw;
    FRESULT res = f_write(&log_file_fat, data, len, &bw);
    if ((res != FR_OK) || (bw != len)) {
        debug("error: f_write %u returned %d", len, res);
    }
}

void log_dump_to_disk(void) {
    FRESULT res = f_open(&log_file_fat, "log.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        debug("error: f_open returned %d", res);
        return;
    }

    rb_dump(&log, log_file_write_callback);

    res = f_close(&log_file_fat);
    if (res != FR_OK) {
        debug("error: f_close returned %d", res);
    }
}

void debug_log_va(bool log, const char *format, va_list args) {
    int l = vsnprintf((char *)line_buff, sizeof(line_buff), format, args);

    if (l < 0) {
        // encoding error
        l = snprintf((char *)line_buff, sizeof(line_buff), "%s: encoding error\r\n", __func__);
    } else if (l >= (ssize_t)sizeof(line_buff)) {
        // not enough space for string
        l = snprintf((char *)line_buff, sizeof(line_buff), "%s: message too long (%d)\r\n", __func__, l);
    }
    if ((l > 0) && (l <= (int)sizeof(line_buff))) {
        usb_cdc_write(line_buff, l);
        serial_write(line_buff, l);

        if (log) {
            add_to_log(line_buff, l);
        }
    }
}

void debug_log(bool log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_log_va(log, format, args);
    va_end(args);
}

void debug_handle_input(const uint8_t *buff, size_t len) {
    (void)buff;

    if (len > 0) {
        got_input = true;
    }
}

void debug_wait_input(const char *format, ...) {
    va_list args;
    va_start(args, format);
    debug_log_va(false, format, args);
    va_end(args);

    got_input = false;
    usb_cdc_set_reroute(true);
    serial_set_reroute(true);

    while (!got_input) {
        main_loop_hw();
    }

    usb_cdc_set_reroute(false);
    serial_set_reroute(false);
}
