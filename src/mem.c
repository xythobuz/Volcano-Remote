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

#include "config.h"
#include "log.h"
#include "mem.h"

#define FLASH_LOCK_TIMEOUT_MS 500
#define FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static struct mem_data data_ram = MEM_DATA_INIT;
static const uint8_t *data_flash = (const uint8_t *)(XIP_BASE + FLASH_OFFSET);

static_assert(sizeof(struct mem_data) < FLASH_SECTOR_SIZE,
              "Config needs to fit inside a flash sector");

void mem_init(void) {
    if (!flash_safe_execute_core_init()) {
        debug("error calling flash_safe_execute_core_init");
    }

    const struct mem_data *flash_ptr = (const struct mem_data *)data_flash;

    if (flash_ptr->version == MEM_VERSION) {
        debug("found matching config (0x%02X)", flash_ptr->version);
        data_ram = *flash_ptr;
    } else {
        debug("invalid config (0x%02X != 0x%02X)", flash_ptr->version, MEM_VERSION);
    }
}

static void mem_write_flash(void *param) {
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);

    // TODO only need to write with length multiple of FLASH_PAGE_SIZE
    flash_range_program(FLASH_OFFSET, param, FLASH_SECTOR_SIZE);
}

void mem_write(void) {
    if (memcmp(&data_ram, data_flash, sizeof(struct mem_data)) == 0) {
        debug("no change, skip write");
        return;
    }

    int r = flash_safe_execute(mem_write_flash, &data_ram, FLASH_LOCK_TIMEOUT_MS);
    if (r != PICO_OK) {
        debug("error calling mem_write_flash: %d", r);
    }
}

struct mem_data *mem_data(void) {
    return &data_ram;
}
