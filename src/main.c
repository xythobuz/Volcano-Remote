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
#include "text.h"
#include "image.h"

int main(void) {
    // required for debug console
    cnsl_init();
    usb_init();

    debug("lcd_init");
    lcd_init();
    draw_splash();
    lcd_set_backlight(0x1000);

    if (watchdog_caused_reboot()) {
        debug("reset by watchdog");
    }

    debug("buttons_init");
    buttons_init();

    // required for LiPo voltage reading
    debug("adc_init");
    adc_init();

    // required for BLE and LiPo voltage reading
    debug("cyw43_arch_init");
    if (cyw43_arch_init()) {
        debug("cyw43_arch_init failed");
    }

    debug("heartbeat_init");
    heartbeat_init();

    debug("ble_init");
    ble_init();

    debug("fat_disk_init");
    fat_disk_init();

    // trigger after 1000ms
    watchdog_enable(1000, 1);

    debug("init done");
    lcd_set_backlight(0x8000);

    while (1) {
        watchdog_update();

        heartbeat_run();
        buttons_run();
        usb_run();
        cnsl_run();

        battery_run();
    }

    return 0;
}
