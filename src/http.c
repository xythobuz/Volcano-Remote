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

#include "config.h"
#include "log.h"
#include "http.h"


void http_init(void) {
    httpd_init();
}

#ifdef DEBUG_DISK_WRITE_SOURCES

#include "lwip/apps/fs.h"
#include <string.h>
#include "pack_data.h"

int fs_open_custom(struct fs_file *file, const char *name) {
    if (strcmp(name, "/src.tar.xz") == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = (const char *)data_tar_xz;
        file->len = data_tar_xz_len;
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    return 0;
}

void fs_close_custom(struct fs_file *file) {
    (void)file;
}

#endif // DEBUG_DISK_WRITE_SOURCES
