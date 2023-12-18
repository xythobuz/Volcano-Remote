/*
 * mem.c
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

#include <string.h>

#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/btstack_flash_bank.h"

#include "config.h"
#include "log.h"
#include "mem.h"

#define FLASH_LOCK_TIMEOUT_MS 500

/*
 * Last two flash pages are used by BTstack.
 * So we use the third-last page for our persistent storage.
 * This is kept clear by our custom linker script.
 */
#define FLASH_OFFSET (PICO_FLASH_BANK_STORAGE_OFFSET - FLASH_SECTOR_SIZE)

struct mem_contents {
    uint8_t version;
    uint32_t checksum;

    struct mem_data data;
};

#define MEM_CONTENTS_INIT { \
    .version = MEM_VERSION, \
    .checksum = 0,          \
    .data = MEM_DATA_INIT,  \
}

static const struct mem_contents data_defaults = MEM_CONTENTS_INIT;
static struct mem_contents data_ram = data_defaults;
static const uint8_t *data_flash = (const uint8_t *)(XIP_BASE + FLASH_OFFSET);

static_assert(sizeof(struct mem_contents) < FLASH_SECTOR_SIZE,
              "Config needs to fit inside a flash sector");

static uint32_t calc_checksum(const struct mem_contents *data) {
    uint32_t c = 0xFFFFFFFF;
    const uint8_t *d = (const uint8_t *)data;

    const size_t offset_checksum = offsetof(struct mem_contents, checksum);
    const size_t size_checksum = sizeof(data->checksum);

    for (size_t i = 0; i < sizeof(struct mem_contents); i++) {
        if ((i >= offset_checksum) && (i < (offset_checksum + size_checksum))) {
            continue;
        }

        // adapted from "Hacker's Delight"
        c ^= d[i];
        for (size_t j = 0; j < 8; j++) {
            uint32_t mask = -(c & 1);
            c = (c >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~c;
}

void mem_load_defaults(void) {
    data_ram = data_defaults;

    data_ram.data.wf_count = wf_default_count;
    for (uint16_t i = 0; i < wf_default_count; i++) {
        data_ram.data.wf[i] = wf_default_data[i];
    }

    // TODO better way to pre-define WiFi credentials
#if defined(DEFAULT_WIFI_SSID) && defined(DEFAULT_WIFI_PASS)
    data_ram.data.net_count = 1;
    strcpy(data_ram.data.net[0].name, DEFAULT_WIFI_SSID);
    strcpy(data_ram.data.net[0].pass, DEFAULT_WIFI_PASS);
#else
    data_ram.data.net_count = 0;
#endif
}

void mem_load(void) {
    mem_load_defaults();

    if (!flash_safe_execute_core_init()) {
        debug("error calling flash_safe_execute_core_init");
    }

    const struct mem_contents *flash_ptr = (const struct mem_contents *)data_flash;

    if (flash_ptr->version == MEM_VERSION) {
        debug("found matching config (0x%02X)", flash_ptr->version);

        uint32_t checksum = calc_checksum(flash_ptr);
        if (checksum != flash_ptr->checksum) {
            debug("invalid checksum (0x%08lX != 0x%08lX)", flash_ptr->checksum, checksum);
        } else {
            debug("loading from flash (0x%08lX)", checksum);
            data_ram = *flash_ptr;
        }
    } else {
        debug("invalid config (0x%02X != 0x%02X)", flash_ptr->version, MEM_VERSION);
    }

#if defined(DEFAULT_WIFI_SSID) && defined(DEFAULT_WIFI_PASS)
    // add default WiFi from #define to flash config, if it is not there yet
    bool found = false;
    for (uint16_t i = 0; i < data_ram.data.net_count; i++) {
        if (strcmp(data_ram.data.net[i].name, DEFAULT_WIFI_SSID) == 0) {
            if (strcmp(data_ram.data.net[i].pass, DEFAULT_WIFI_PASS) != 0) {
                debug("warning: restoring wifi password for '%s'", DEFAULT_WIFI_SSID);
                strcpy(data_ram.data.net[i].pass, DEFAULT_WIFI_PASS);
            }
            found = true;
            break;
        }
    }
    if ((!found) && (data_ram.data.net_count < WIFI_MAX_NET_COUNT)) {
        debug("info: adding wifi password for '%s'", DEFAULT_WIFI_SSID);
        strcpy(data_ram.data.net[data_ram.data.net_count].name, DEFAULT_WIFI_SSID);
        strcpy(data_ram.data.net[data_ram.data.net_count].pass, DEFAULT_WIFI_PASS);
        data_ram.data.net_count++;
    }
#endif
}

static void mem_write_flash(void *param) {
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);

    // TODO only need to write with length multiple of FLASH_PAGE_SIZE
    flash_range_program(FLASH_OFFSET, param, FLASH_SECTOR_SIZE);
}

void mem_write(void) {
    if (memcmp(&data_ram, data_flash, sizeof(struct mem_contents)) == 0) {
        debug("no change, skip write");
        return;
    }

    data_ram.checksum = calc_checksum(&data_ram);

    debug("writing new data (0x%08lX)", data_ram.checksum);
    int r = flash_safe_execute(mem_write_flash, &data_ram, FLASH_LOCK_TIMEOUT_MS);
    if (r != PICO_OK) {
        debug("error calling mem_write_flash: %d", r);
    }
}

struct mem_data *mem_data(void) {
    return &data_ram.data;
}
