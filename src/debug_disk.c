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

#include "pack_data.h"

static FATFS fs;
static bool mounted = false;

void debug_disk_init(void) {
    if (debug_disk_mount() != 0) {
        debug("error mounting disk");
        return;
    }

    // maximum length: 11 bytes
    f_setlabel("DEBUG DISK");

    FIL file;
    FRESULT res = f_open(&file, "README.md", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        debug("error: f_open returned %d", res);
    } else {
        char readme[1024];
        size_t pos = 0;
        pos += snprintf(readme + pos, 1024 - pos, "# Volcano Remote Control Gadget\r\n");
        pos += snprintf(readme + pos, 1024 - pos, "\r\n");
        pos += snprintf(readme + pos, 1024 - pos, "Project by Thomas Buck <thomas@xythobuz.de>\r\n");
        pos += snprintf(readme + pos, 1024 - pos, "Licensed under GPLv3.\r\n");
        pos += snprintf(readme + pos, 1024 - pos, "See included 'src.tar.xz' for sources.\r\n");
        pos += snprintf(readme + pos, 1024 - pos, "Repo at https://git.xythobuz.de/thomas/sb-py\r\n");

        size_t len = strlen(readme);
        UINT bw;
        res = f_write(&file, readme, len, &bw);
        if ((res != FR_OK) || (bw != len)) {
            debug("error: f_write returned %d", res);
        }

        res = f_close(&file);
        if (res != FR_OK) {
            debug("error: f_close returned %d", res);
        }
    }

    res = f_open(&file, "src.tar.xz", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        debug("error: f_open returned %d", res);
    } else {
        UINT bw;
        UINT len = 0;
        while (1) {
            res = f_write(&file, data_tar_xz + len, data_tar_xz_len - len, &bw);
            len += bw;
            if (res != FR_OK) {
                debug("error: f_write returned %d", res);
                break;
            } else if (bw == 0) {
                debug("error: f_write did not write");
                break;
            } else if (bw == data_tar_xz_len) {
                break;
            }
        }

        res = f_close(&file);
        if (res != FR_OK) {
            debug("error: f_close returned %d", res);
        }
    }

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
