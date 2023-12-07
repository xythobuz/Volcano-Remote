/*
 * wifi.c
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/adc/read_vsys/power_status.c
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

#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "config.h"
#include "log.h"
#include "mem.h"
#include "wifi.h"

enum wifi_state {
    WS_IDLE = 0,
    WS_SCAN,
    WS_CONNECT,
    WS_READY,
};

static enum wifi_state state = WS_IDLE;

static void wifi_connect(const char *ssid, const char *pw, uint32_t auth) {
    debug("connecting to '%s'", ssid);

    // https://github.com/raspberrypi/pico-sdk/issues/1413
    uint32_t a = 0;
    if (auth & 4) {
        a = CYW43_AUTH_WPA2_AES_PSK;
    } else if (auth & 2) {
        a = CYW43_AUTH_WPA_TKIP_PSK;
    }

    int r = cyw43_arch_wifi_connect_async(ssid, pw, a);
    if (r != 0) {
        debug("failed to connect %d", r);
    } else {
        state = WS_CONNECT;
    }
}

static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    (void)env;
    cyw43_thread_enter();

    if (result && (state == WS_SCAN)) {
        for (int i = 0; i < mem_data()->net_count; i++) {
            if ((strlen(mem_data()->net[i].name) == result->ssid_len)
                 && (memcmp(mem_data()->net[i].name, result->ssid, result->ssid_len) == 0)) {
                wifi_connect(mem_data()->net[i].name,
                             mem_data()->net[i].pass,
                             result->auth_mode);
                break;
            }
        }
    }

    cyw43_thread_exit();
    return 0;
}

static void wifi_scan(void) {
    debug("starting scan");
    cyw43_wifi_scan_options_t scan_options = {0};
    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
    if (err != 0) {
        debug("error %d", err);
    }
    state = WS_SCAN;
}

void wifi_init(void) {
    if (state != WS_IDLE) {
        debug("invalid state %d", state);
        return;
    }

    cyw43_thread_enter();
    cyw43_arch_enable_sta_mode();
    wifi_scan();
    cyw43_thread_exit();
}

void wifi_deinit(void) {
    cyw43_thread_enter();
    cyw43_arch_disable_sta_mode();
    state = WS_IDLE;
    cyw43_thread_exit();
}

bool wifi_initialized(void) {
    return (state != WS_IDLE);
}

const char *wifi_state(void) {
    switch (state) {
    case WS_IDLE:
        return "Disabled";

    case WS_SCAN:
        return "Scanning";

    case WS_CONNECT:
        return "Connecting";

    case WS_READY:
        return "Connected";
    }

    return NULL;
}

void wifi_run(void) {
    cyw43_thread_enter();

    if (state == WS_SCAN) {
        if (!cyw43_wifi_scan_active(&cyw43_state)) {
            debug("restarting scan");
            wifi_scan();
        }
    } else if (state == WS_CONNECT) {
        if (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_JOIN) {
            debug("joined network");
            state = WS_READY;
        }
    }

    cyw43_thread_exit();
}
