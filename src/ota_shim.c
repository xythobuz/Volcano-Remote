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

#include "pico/cyw43_arch.h"

#include "config.h"
#include "lcd.h"
#include "log.h"
#include "mem.h"
#include "wifi.h"

int picowota_network_init(void) {
    debug("mem_load");
    mem_load();

    debug("lcd_init");
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
        cyw43_arch_poll();
        wifi_run();

        const char *state = wifi_state();
        if (state != prev) {
            prev = state;
            debug("new state: %s", state);
        }

        log_dump_to_lcd();

        // TODO open AP when timed out?
    }

    debug("wifi ready");
    log_dump_to_lcd();

    return 0;
}

void picowota_network_deinit(void) {
    debug("wifi_deinit");
    wifi_deinit();
}
