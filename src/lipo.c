/*
 * lipo.c
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/adc/read_vsys/power_status.c
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

#include "stdbool.h"
#include "hardware/adc.h"

#include "config.h"
#include "lipo.h"

#if CYW43_USES_VSYS_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef PICO_POWER_SAMPLE_COUNT
#define PICO_POWER_SAMPLE_COUNT 3
#endif

// Pin used for ADC 0
#define PICO_FIRST_ADC_PIN 26

static const float full_battery = 4.1f;
static const float empty_battery = 3.2f;

bool lipo_charging(void) {
#if defined CYW43_WL_GPIO_VBUS_PIN
    return cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#elif defined PICO_VBUS_PIN
    gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
    return gpio_get(PICO_VBUS_PIN);
#else
#error "No VBUS Pin available!"
#endif
}

float lipo_voltage(void) {
#ifndef PICO_VSYS_PIN
#error "No VSYS Pin available!"
#endif

#if CYW43_USES_VSYS_PIN
    cyw43_thread_enter();
    // Make sure cyw43 is awake
    cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#endif

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);

    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // We seem to read low values initially - this seems to fix it
    int ignore_count = PICO_POWER_SAMPLE_COUNT;
    while (!adc_fifo_is_empty() || ignore_count-- > 0) {
        (void)adc_fifo_get_blocking();
    }

    // read vsys
    uint32_t vsys = 0;
    for(int i = 0; i < PICO_POWER_SAMPLE_COUNT; i++) {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
    }

    adc_run(false);
    adc_fifo_drain();

    vsys /= PICO_POWER_SAMPLE_COUNT;

#if CYW43_USES_VSYS_PIN
    cyw43_thread_exit();
#endif

    // Generate voltage
    const float conversion_factor = 3.3f / (1 << 12);
    return vsys * 3 * conversion_factor;
}

float lipo_percentage(float voltage) {
    float percentage = 100.0f * ((voltage - empty_battery) / (full_battery - empty_battery));
    if (percentage >= 100.0f) {
        percentage = 100.0f;
    } else if (percentage < 0.0f) {
        percentage = 0.0f;
    }
    return percentage;
}
