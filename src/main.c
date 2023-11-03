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

int main(void) {
    heartbeat_init();
    buttons_init();

    cnsl_init();
    usb_init();

    if (watchdog_caused_reboot()) {
        debug("reset by watchdog");
    }

    // required for LiPo voltage reading
    adc_init();

    if (cyw43_arch_init()) {
        debug("cyw43_arch failed");
    }

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
