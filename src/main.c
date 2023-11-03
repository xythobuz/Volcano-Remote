/*
 * main.c
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

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"

#include "config.h"
#include "util.h"
#include "console.h"
#include "log.h"
#include "usb.h"
#include "fat_disk.h"
#include "buttons.h"
#include "ble.h"
#include "lcd.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtrigraphs"
#include "logo.h"
#pragma GCC diagnostic pop

static void draw_splash(void) {
    char *data = logo_rgb_data;
    for (uint x = 0; x < logo_width; x++) {
        for (uint y = 0; y < logo_height; y++) {
            uint32_t pixel[3];
            HEADER_PIXEL(data, pixel);

            uint32_t color = (pixel[0] >> 3) << 11;
            color |= (pixel[1] >> 2) << 5;
            color |= pixel[2] >> 3;
            lcd_write_point(240 - x, y, color);
        }
    }
}

int main(void) {
    heartbeat_init();
    buttons_init();
    cnsl_init();
    usb_init();

    debug("lcd_init");
    lcd_init();
    draw_splash();
    lcd_set_backlight(0x8000);

    if (watchdog_caused_reboot()) {
        debug("reset by watchdog");
    }

    // required for LiPo voltage reading
    adc_init();

    // required for BLE and LiPo voltage reading
    debug("cyw43_arch_init");
    if (cyw43_arch_init()) {
        debug("cyw43_arch_init failed");
    }

    debug("ble_init");
    ble_init();

    debug("fat_disk_init");
    fat_disk_init();

    // trigger after 1000ms
    watchdog_enable(1000, 1);

    debug("init done");

    while (1) {
        watchdog_update();

        heartbeat_run();
        buttons_run();
        usb_run();
        cnsl_run();
    }

    return 0;
}
