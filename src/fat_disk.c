/* 
 * fat_disk.c
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
#include <stdlib.h>

#include "pico/stdlib.h"
#include "ff.h"
#include "diskio.h"

#include "config.h"
#include "cache.h"
#include "log.h"
#include "debug_disk.h"

/*
 * FatFS ffsystem.c
 */

void* ff_memalloc(UINT msize) {
    return malloc((size_t)msize);
}

void ff_memfree(void* mblock) {
    free(mblock);
}

/*
 * FatFS diskio.c
 */

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != 0) {
        debug("invalid drive number %d", pdrv);
        return STA_NODISK;
    }

    return 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != 0) {
        debug("invalid drive number %d", pdrv);
        return STA_NODISK;
    }

    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != 0) {
        debug("invalid drive number %d", pdrv);
        return RES_PARERR;
    }

    if ((sector + count) > DISK_BLOCK_COUNT) {
        debug("invalid read ((%lu + %u) > %u)", sector, count, DISK_BLOCK_COUNT);
        return RES_ERROR;
    }

    ssize_t r = cache_read(buff, sector * DISK_BLOCK_SIZE, count * DISK_BLOCK_SIZE);
    return (r == (ssize_t)(count * DISK_BLOCK_SIZE)) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != 0) {
        debug("invalid drive number %d", pdrv);
        return RES_PARERR;
    }

    if ((sector + count) > DISK_BLOCK_COUNT) {
        debug("invalid read ((%lu + %u) > %u)", sector, count, DISK_BLOCK_COUNT);
        return RES_ERROR;
    }

    ssize_t r = cache_write(buff, sector * DISK_BLOCK_SIZE, count * DISK_BLOCK_SIZE);
    return (r == (ssize_t)(count * DISK_BLOCK_SIZE)) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv != 0) {
        debug("invalid drive number %d", pdrv);
        return RES_PARERR;
    }

    switch (cmd) {
        case GET_SECTOR_COUNT:
            *((LBA_t *)buff) = DISK_BLOCK_COUNT;
            break;

        case GET_SECTOR_SIZE:
            *((WORD *)buff) = DISK_BLOCK_SIZE;
            break;

        case GET_BLOCK_SIZE:
            *((DWORD *)buff) = 1; // non flash memory media
            break;
    }

    return RES_OK;
}
