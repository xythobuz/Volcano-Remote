/*
 * http.c
 *
 * https://github.com/krzmaz/pico-w-webserver-example
 * https://github.com/lwip-tcpip/lwip/blob/master/contrib/examples/httpd/genfiles_example/genfiles_example.c
 * https://www.nongnu.org/lwip/2_1_x/group__httpd.html
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

#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "ff.h"

#include <string.h>

#include "config.h"
#include "log.h"
#include "debug_disk.h"
#include "http.h"

void http_init(void) {
    httpd_init();
}

int fs_open_custom(struct fs_file *file, const char *name) {
    debug("'%s'", name);

    // TODO only do this when not mounted via USB
    debug_disk_mount();

    FIL f;
    FRESULT r = f_open (&f, name, FA_READ);
    if (r == FR_OK) {
        FSIZE_t len = f_size(&f);
        char *data = malloc(len);
        if (!data) {
            debug("error: not enough memory");
            f_close(&f);
            debug_disk_unmount();
            return 0;
        }

        UINT read_count = 0;
        r = f_read(&f, data, len, &read_count);
        if ((r != FR_OK) || (read_count != len)) {
            debug("invalid read: %d %d %ld", r, read_count, len);
        }

        memset(file, 0, sizeof(struct fs_file));
        file->data = data;
        file->len = len;
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;

        f_close(&f);
        debug_disk_unmount();
        return 1;
    }

    debug_disk_unmount();
    return 0;
}

void fs_close_custom(struct fs_file *file) {
    debug("len=%d", file->len);

    if (file && file->data) {
        free((void *)file->data);
        file->data = NULL;
    }
}
