/*
 * ota_shim.c
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

#include <stdarg.h>
#include <stdio.h>

#include "pico/cyw43_arch.h"

#include "config.h"
#include "buttons.h"
#include "lcd.h"
#include "log.h"
#include "lcd.h"
#include "textbox.h"
#include "mem.h"
#include "util.h"
#include "usb_descriptors.h"
#include "wifi.h"

static int16_t lcd_off = 0;
static size_t text_window = 420; // TODO
static size_t text_off = 0;
static size_t prev_len = 0;
static bool redraw = false;

static void lcd_write(const void *buf, size_t len) {
    char tmp[len + 1];
    memcpy(tmp, buf, len);
    tmp[len] = '\0';
    lcd_off = text_box(tmp, false,
                       "fixed_5x8",
                       0, LCD_WIDTH,
                       lcd_off, LCD_HEIGHT - lcd_off,
                       0);
}

static void log_dump_to_lcd(void) {
    // TODO length is not good as indicator.
    // TODO will stop working when log buffer is filled.
    size_t len = rb_len(log_get());
    if ((!redraw) && (len == prev_len)) {
        return;
    }
    prev_len = len;
    redraw = false;

    lcd_off = 0;

    text_off = 0;
    if (len > text_window) {
        text_off = len - text_window;
    }

    rb_dump(log_get(), lcd_write, text_off);
}

static void ota_buttons(enum buttons btn, bool state) {
    if (state && (btn == BTN_Y)) {
        reset_to_main();
    } else if (state && (btn == BTN_LEFT)) {
        if (text_window > 10) {
            text_window -= 10;
            redraw = true;
        }
    } else if (state && (btn == BTN_RIGHT)) {
        if (text_window < 1000) {
            text_window += 10;
            redraw = true;
        }
    } else if (state && (btn == BTN_UP)) {
        if (text_off > 10) {
            text_off -= 10;
            redraw = true;
        }
    } else if (state && (btn == BTN_DOWN)) {
        if (text_off < (prev_len - 10)) {
            text_off += 10;
            redraw = true;
        }
    }
}

void picowota_printf_init(void) { }

void picowota_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    debug_log_va(true, format, args);
    va_end(args);

    log_dump_to_lcd();
}

void picowota_poll(void) {
    buttons_run();
    cyw43_arch_poll();
    wifi_run();
    log_dump_to_lcd();
}

int picowota_init(void) {
    usb_descriptor_init_id();
    buttons_init();
    buttons_callback(ota_buttons);
    mem_load();
    lcd_init();
    lcd_set_backlight(mem_data()->backlight);
    log_dump_to_lcd();

    debug("cyw43_arch_init");
    log_dump_to_lcd();

    if (cyw43_arch_init_with_country(COUNTRY_CODE)) {
        debug("failed to init cyw43");
        log_dump_to_lcd();

        return 1;
    }

    debug("wifi_init");
    log_dump_to_lcd();

    wifi_init();

    const char *prev = NULL;
    while (!wifi_ready()) {
        const char *state = wifi_state();
        if (state != prev) {
            prev = state;
            debug("new state: %s", state);
        }

        picowota_poll();
    }

    debug("wifi ready");
    picowota_poll();

    return 0;
}

void picowota_deinit(void) {
    debug("wifi_deinit");
    log_dump_to_lcd();

    wifi_deinit();
}
