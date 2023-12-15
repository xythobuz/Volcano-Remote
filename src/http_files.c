/*
 * http_files.c
 *
 * Based on BittyHTTP example.
 *
 * Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
 * Copyright (c) 2019 Paul Hutchinson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>

#include "WebServer.h"

#include "config.h"
#include "log.h"
#include "http.h"

struct FileInfo {
    const char *Filename; // With Path
    bool Dynamic;
    const char **Cookies;
    const char **Gets;
    const char **Posts;
    void (*WriteFile)(struct WebServer *Web);
};

void File_Root(struct WebServer *Web);

static struct FileInfo m_Files[] =  {
    // Filename, Dynamic, Cookies, Gets, Posts, Callback
    {       "/",   false,    NULL, NULL,  NULL, File_Root },
};

bool FS_GetFileProperties(const char *Filename, struct WSPageProp *PageProp) {
    PageProp->FileID = 0;

    for (size_t r = 0; r < sizeof(m_Files) / sizeof(struct FileInfo); r++) {
        if (strcmp(Filename, m_Files[r].Filename) != 0) {
            continue;
        }

        PageProp->FileID = (uintptr_t)&m_Files[r];
        PageProp->DynamicFile = m_Files[r].Dynamic;
        PageProp->Cookies = m_Files[r].Cookies;
        PageProp->Gets = m_Files[r].Gets;
        PageProp->Posts = m_Files[r].Posts;

        debug("serving '%s'", Filename);
        return true;
    }

    debug("unknown file '%s'", Filename);
    return false;
}

void FS_SendFile(struct WebServer *Web, uintptr_t FileID) {
    struct FileInfo *File = (struct FileInfo *)FileID;
    if ((Web == NULL) || (File == NULL)) {
        debug("invalid param");
        return;
    }

    File->WriteFile(Web);
}

t_ElapsedTime ReadElapsedClock(void) {
    return to_ms_since_boot(get_absolute_time()) / 1000;
}

const char RootHTML[]=
"<!DOCTYPE html>"
"<html>"
"<head>"
    "<title>Volcano Remote</title>"
"</head>"
"<body>"
    "<h1>Hello World</h1>"
"</body>"
"</html>";

void File_Root(struct WebServer *Web) {
    WS_WriteWhole(Web,RootHTML, sizeof(RootHTML) - 1);
}
