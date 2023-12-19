/*
 * mem.h
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

#ifndef __MEM_H__
#define __MEM_H__

#include "hardware/flash.h"
#include "pico/btstack_flash_bank.h"

#include <stdint.h>
#include <stdbool.h>

#include "workflow.h"
#include "wifi.h"

/*
 * Last two flash pages are used by BTstack.
 * So we use the third-last page for our persistent storage.
 * This is kept clear by our custom linker script.
 */
#define EEPROM_FLASH_OFFSET (PICO_FLASH_BANK_STORAGE_OFFSET - FLASH_SECTOR_SIZE)

// to migrate settings when struct changes between releases
#define MEM_VERSION 0

struct mem_data {
    // wifi networks
    // should stay at beginning, for bootloader
    uint16_t net_count;
    struct net_credentials net[WIFI_MAX_NET_COUNT];

    // settings
    uint16_t backlight;
    bool wf_auto_connect;
    bool enable_wifi;

    // workflows
    uint16_t wf_count;
    struct workflow wf[WF_MAX_FLOWS];
};

// workflows are assigned in mem_init()
#define MEM_DATA_INIT {           \
    .backlight = (0xFF00 >> 1),   \
    .wf_auto_connect = false,     \
    .enable_wifi = false,         \
    .wf_count = 0,                \
    .net_count = 0,               \
}

void mem_load(void);
void mem_write(void);
struct mem_data *mem_data(void);
void mem_load_defaults(void);

#endif // __MEM_H__
