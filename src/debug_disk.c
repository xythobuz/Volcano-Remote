/*
 * debug_disk.c
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
#include <stdio.h>

#include "pico/stdlib.h"
#include "ff.h"

#include "config.h"
#include "log.h"
#include "debug_disk.h"

static FATFS fs;
static bool mounted = false;

void debug_disk_init_log(void) {
    if (debug_disk_mount() != 0) {
        debug("error mounting disk");
        return;
    }

    log_dump_to_disk();

    if (debug_disk_unmount() != 0) {
        debug("error unmounting disk");
    }
}

int debug_disk_mount(void) {
    if (mounted) {
        debug("already mounted");
        return 0;
    }

    FRESULT res = f_mount(&fs, "", 0);
    if (res != FR_OK) {
        debug("error: f_mount returned %d", res);
        mounted = false;
        return -1;
    }

    mounted = true;
    return 0;
}

int debug_disk_unmount(void) {
    if (!mounted) {
        debug("already unmounted");
        return 0;
    }

    FRESULT res = f_mount(0, "", 0);
    if (res != FR_OK) {
        debug("error: f_mount returned %d", res);
        return -1;
    }

    mounted = false;
    return 0;
}
