/*
 * util.c
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

#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "picowota/reboot.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif // CYW43_WL_GPIO_LED_PIN

#include "config.h"
#include "log.h"
#include "util.h"

#define HEARTBEAT_INTERVAL_MS 500

void heartbeat_init(void) {
#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
#endif // PICO_DEFAULT_LED_PIN
#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#endif // CYW43_WL_GPIO_LED_PIN
}

void heartbeat_run(void) {
#if defined(PICO_DEFAULT_LED_PIN) || defined(CYW43_WL_GPIO_LED_PIN)
    static uint32_t last_heartbeat = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now >= (last_heartbeat + HEARTBEAT_INTERVAL_MS)) {
        last_heartbeat = now;
#ifdef PICO_DEFAULT_LED_PIN
        gpio_xor_mask(1 << PICO_DEFAULT_LED_PIN);
#endif // PICO_DEFAULT_LED_PIN
#ifdef CYW43_WL_GPIO_LED_PIN
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN));
#endif // CYW43_WL_GPIO_LED_PIN
    }
#endif // defined(PICO_DEFAULT_LED_PIN) || defined(CYW43_WL_GPIO_LED_PIN)
}

bool str_startswith(const char *str, const char *start) {
    size_t l = strlen(start);
    if (l > strlen(str)) {
        return false;
    }
    return (strncmp(str, start, l) == 0);
}

int32_t convert_two_complement(int32_t b) {
    if (b & 0x8000) {
        b = -1 * ((b ^ 0xffff) + 1);
    }
    return b;
}

void reset_to_bootloader(void) {
#ifdef PICO_DEFAULT_LED_PIN
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
#else // ! PICO_DEFAULT_LED_PIN
    reset_usb_boot(0, 0);
#endif // PICO_DEFAULT_LED_PIN
}

void reset_to_ota(void) {
    picowota_reboot(true);
}

void reset_to_main(void) {
    picowota_reboot(false);
}

void hexdump(const uint8_t *buff, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        for (size_t j = 0; (j < 16) && ((i + j) < len); j++) {
            print("0x%02X", buff[i + j]);
            if ((j < 15) && ((i + j) < (len - 1))) {
                print(" ");
            }
        }
        println();
    }
}

float map(float value, float leftMin, float leftMax, float rightMin, float rightMax) {
    float leftSpan = leftMax - leftMin;
    float rightSpan = rightMax - rightMin;
    float valueScaled = (value - leftMin) / leftSpan;
    return rightMin + (valueScaled * rightSpan);
}
