/*
 * http.c
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

#include "WebServer.h"

#include "config.h"
#include "log.h"
#include "http.h"

void http_init(void) {
    SocketsCon_InitSocketConSystem();
    WS_Init();

    if (!WS_Start(80)) {
        debug("failed to start web server");
    } else {
        debug("listening on :80");
    }
}

void http_deinit(void) {
    WS_Shutdown();
    SocketsCon_ShutdownSocketConSystem();
}

void http_run(void) {
    WS_Tick();
}
