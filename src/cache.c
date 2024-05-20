/*
 * cache.c
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

#include <stdbool.h>
#include <string.h>

#include "pico/flash.h"

#include "config.h"
#include "log.h"
#include "mem.h"
#include "cache.h"

#define CACHE_MAX_AGE_MS (5 * 1000)

#define PAGE_SIZE FLASH_SECTOR_SIZE // 4K
#define CACHE_ENTRIES 10 // 40K in RAM

// 384 * 512 = 196608 bytes = 48 Pages (Sectors) in Flash
#define DISK_SIZE (DISK_BLOCK_SIZE * DISK_BLOCK_COUNT)
#define DISK_PAGES (DISK_SIZE / PAGE_SIZE)

static_assert((DISK_SIZE % PAGE_SIZE) == 0, "Disk blocks need to fit cleanly into flash pages.");

// PICO_FLASH_SIZE_BYTES is 2MB = 512 * 4K pages
// BTstack uses the last two pages, we use one more for EEPROM config.
// So 509 pages remain, minus the 48 pages for the FAT disk.
// --> 461 pages remain for bootloader and application.
// --> 461 * 4096 = 1888256 bytes
#define CACHE_FLASH_OFFSET (EEPROM_FLASH_OFFSET - (DISK_PAGES * PAGE_SIZE))
static_assert(DISK_PAGES == 48, "TODO conversions are hard-coded currently");

struct cache_entry {
    bool set;
    size_t page;
    bool dirty;
    uint32_t age;
    uint8_t buff[PAGE_SIZE];
};

static struct cache_entry cache[CACHE_ENTRIES] = {0};
static const uint8_t *disk = (const uint8_t *)(XIP_BASE + CACHE_FLASH_OFFSET);

void cache_init(void) {
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        cache[i].set = false;
    }
}

void cache_status(void) {
    size_t count = 0;

    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            continue;
        }

        println("Cache Entry %d:", count);
        println("  Page: %d", cache[i].page);
        println("  Dirty: %s", cache[i].dirty ? "Yes" : "No");

        uint32_t now = to_ms_since_boot(get_absolute_time());
        println("  Age: %.1fs", (now - cache[i].age) / 1000.0f);

        count++;
    }

    println("Total entries: %d", count);
}

struct cache_write_data {
    uint32_t addr;
    uint8_t *buff;
};

static void cache_write_flash(void *param) {
    struct cache_write_data *tmp = param;
    flash_range_erase(tmp->addr, PAGE_SIZE);
    flash_range_program(tmp->addr, tmp->buff, PAGE_SIZE);
}

static void cache_flush(size_t i) {
    // after this the cache entry is gone
    cache[i].set = false;

    // if it is not actually modified, we can bail out here
    if (!cache[i].dirty) {
        return;
    }

    // now actually write contents back to flash
    uint32_t addr = CACHE_FLASH_OFFSET + (cache[i].page * PAGE_SIZE);
    debug("flushing entry %d page %d at 0x%08" PRIX32, i, cache[i].page, addr);

    struct cache_write_data tmp = { .addr = addr, .buff = cache[i].buff };
    int r = flash_safe_execute(cache_write_flash, &tmp, FLASH_LOCK_TIMEOUT_MS);
    if (r != PICO_OK) {
        debug("error calling cache_write_flash: %d", r);
    }
}

void cache_sync(void) {
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            continue;
        }

        // just flush out all available entries
        cache_flush(i);
    }
}

void cache_run(void) {
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            continue;
        }

        // only flush out entries that are too old
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - cache[i].age) >= CACHE_MAX_AGE_MS) {
            cache_flush(i);
        }
    }
}

static ssize_t cache_read_single(uint8_t *buf, size_t page, size_t off, size_t len) {
    if (page >= DISK_PAGES) {
        debug("error: invalid page %d", page);
        return -1;
    }

    if ((off + len) > PAGE_SIZE) {
        debug("error: too long %d %d", off, len);
        return -1;
    }

    // is it in the cache?
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            continue;
        }

        if (cache[i].page != page) {
            continue;
        }

        // it is in the cache!
        memcpy(buf, cache[i].buff + off, len);
        return len;
    }

    // not in cache, read directly from flash
    memcpy(buf, disk + (page * PAGE_SIZE) + off, len);
    return len;
}

static ssize_t write_into_cache(size_t idx, const uint8_t *buf, size_t off, size_t len) {
    bool changed = false;
    for (size_t i = 0; i < len; i++) {
        if (cache[idx].buff[off + i] != buf[i]) {
            changed = true;
        }

        cache[idx].buff[off + i] = buf[i];
    }

    if (changed) {
        cache[idx].dirty = true;
        cache[idx].age = to_ms_since_boot(get_absolute_time());
    }

    return len;
}

static ssize_t cache_write_single(const uint8_t *buf, size_t page, size_t off, size_t len) {
    if (page >= DISK_PAGES) {
        debug("error: invalid page %d", page);
        return -1;
    }

    if ((off + len) > PAGE_SIZE) {
        debug("error: too long %d %d", off, len);
        return -1;
    }

    // is it in the cache?
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            continue;
        }

        if (cache[i].page != page) {
            continue;
        }

        // it is in the cache!
        return write_into_cache(i, buf, off, len);
    }

    // not in cache yet, find free cache entry
    ssize_t free = -1, oldest = -1;
    uint32_t min_age = 0xFFFFFFFF;
    for (size_t i = 0; i < CACHE_ENTRIES; i++) {
        if (!cache[i].set) {
            if (free < 0) {
                free = i;
            }
        }

        if (cache[i].age < min_age) {
            min_age = cache[i].age;
            oldest = i;
        }
    }

    if (free < 0) {
        // flush oldest current entry and use its place
        if (oldest < 0) {
            debug("error, no oldest entry?!");
            return -1;
        }

        cache_flush(oldest);
        free = oldest;
    }

    // populate new cache entry
    cache[free].set = true;
    cache[free].page = page;
    cache[free].dirty = false;
    cache[free].age = to_ms_since_boot(get_absolute_time());
    memcpy(cache[free].buff, disk + (page * PAGE_SIZE), PAGE_SIZE);

    debug("add cache %d for page %d", free, page);

    // perform write operation into cache entry
    return write_into_cache(free, buf, off, len);
}

ssize_t cache_read(uint8_t *buf, size_t addr, size_t len) {
    size_t page = addr / PAGE_SIZE;
    size_t off = addr % PAGE_SIZE;
    size_t count = 0;

    while ((off + len) > PAGE_SIZE) {
        debug("split cache page read");

        size_t chunk = PAGE_SIZE - off;
        ssize_t r = cache_read_single(buf + count, page, off, chunk);
        if (r < 0) {
            return r;
        }
        count += r;
        len -= r;
        off = 0;
        page++;
    }

    ssize_t r = cache_read_single(buf + count, page, off, len);
    if (r < 0) {
        return r;
    }
    count += r;
    return count;
}

ssize_t cache_write(const uint8_t *buf, size_t addr, size_t len) {
    size_t page = addr / PAGE_SIZE;
    size_t off = addr % PAGE_SIZE;
    size_t count = 0;

    while ((off + len) > PAGE_SIZE) {
        debug("split cache page write");

        size_t chunk = PAGE_SIZE - off;
        ssize_t r = cache_write_single(buf + count, page, off, chunk);
        if (r < 0) {
            return r;
        }
        count += r;
        len -= r;
        off = 0;
        page++;
    }

    ssize_t r = cache_write_single(buf + count, page, off, len);
    if (r < 0) {
        return r;
    }
    count += r;
    return count;
}
