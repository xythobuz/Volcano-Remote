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
#include "usb_msc.h"
#include "debug_disk.h"
#include "buttons.h"
#include "ble.h"
#include "lcd.h"
#include "text.h"
#include "image.h"
#include "mem.h"
#include "state.h"
#include "serial.h"
#include "workflow.h"
#include "wifi.h"
#include "http.h"
#include "cache.h"
#include "main.h"

void main_loop_hw(void) {
    watchdog_update();
    usb_run();
    serial_run();
    heartbeat_run();

    if (lcd_get_backlight() != mem_data()->backlight) {
        lcd_set_backlight(mem_data()->backlight);
    }

    networking_run();
    cache_run();
}

void networking_init(void) {
    debug("wifi_init");
    wifi_init();

    debug("http_init");
    http_init();
}

void networking_deinit(void) {
    debug("wifi_deinit");
    wifi_deinit();
}

void networking_run(void) {
    wifi_run();
}

int main(void) {
    watchdog_enable(WATCHDOG_PERIOD_MS, 1);

    // required for debug console
    cnsl_init();
#ifndef NDEBUG
    serial_init();
#endif
    usb_init();

    debug("mem_load");
    mem_load();

    debug("lcd_init");
    lcd_init();

    watchdog_update();

    debug("draw_splash");
    draw_splash();
    lcd_set_backlight(mem_data()->backlight);

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
    if (cyw43_arch_init_with_country(COUNTRY_CODE)) {
        debug("cyw43_arch_init failed");
        lcd_set_backlight(0x00FF);
        while (1) {}
    }

    watchdog_update();

    debug("heartbeat_init");
    heartbeat_init();

    debug("ble_init");
    ble_init();

    debug("cache_init");
    cache_init();

#ifdef AUTO_MOUNT_MASS_STORAGE
    msc_set_medium_available(true);
#endif // AUTO_MOUNT_MASS_STORAGE

    watchdog_update();

    debug("init done");
    battery_run();

    // wait for BLE stack to be ready before using it
    debug("wait for bt stack");
    while (!ble_is_ready()) {
        sleep_ms(1);
    }

    if (mem_data()->enable_wifi) {
        debug("networking_init");
        networking_init();
    } else {
        debug("wifi not enabled");
    }

    debug("starting app");
    state_switch(STATE_SCAN);

    while (1) {
        main_loop_hw();
        buttons_run();
        cnsl_run();
        battery_run();
        state_run();
        wf_run();
    }

    return 0;
}
