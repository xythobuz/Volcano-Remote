/*
 * wifi.c
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
#include "lwip/tcp.h"
#include "dhcpserver.h"

#include "config.h"
#include "log.h"
#include "mem.h"
#include "usb_descriptors.h"
#include "wifi.h"

#define CONNECT_TIMEOUT_MS (8UL * 1000UL)
#define SCAN_AP_TIMEOUT_MS (20UL * 1000UL)

#define WIFI_AP_SSID_PREFIX "Volcano-"
#define WIFI_AP_SSID_LEN 4
#define WIFI_AP_PASS_LEN 8
static_assert((WIFI_AP_SSID_LEN + WIFI_AP_PASS_LEN) <= (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES),
              "SSID and Password parts for AP need to fit Pico serial number");

enum wifi_state {
    WS_IDLE = 0,
    WS_SCAN,
    WS_CONNECT,
    WS_WAIT_FOR_IP,
    WS_READY,
};

static enum wifi_state state = WS_IDLE;
static uint32_t start_scan_time = 0;
static uint32_t start_connect_time = 0;
static uint32_t start_ip_time = 0;
static dhcp_server_t dhcp_server;
static bool enabled_ap = false;
static char curr_ssid[WIFI_MAX_NAME_LEN + 1] = {0};
static char curr_pass[WIFI_MAX_PASS_LEN + 1] = {0};

struct wifi_tcp_cache {
    bool in_use;
    bool clear; // TODO
    ip4_addr_t ip;
    uint16_t port;
    struct tcp_pcb *tpcb;
    char data[512];
    unsigned int len;
};

static struct wifi_tcp_cache cache[4] = {0};

static void wifi_ap(void) {
    cyw43_thread_enter();

    // last N chars of serial for ssid and password
    const size_t prefix_len = strlen(WIFI_AP_SSID_PREFIX);
    char wifi_ssid[prefix_len + WIFI_AP_SSID_LEN + 1];
    memcpy(wifi_ssid, WIFI_AP_SSID_PREFIX, prefix_len);
    memcpy(wifi_ssid + prefix_len,
           string_pico_serial + (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES) - WIFI_AP_SSID_LEN,
           WIFI_AP_SSID_LEN);
    wifi_ssid[prefix_len + WIFI_AP_SSID_LEN] = '\0';
    strncpy(curr_ssid, wifi_ssid, WIFI_MAX_NAME_LEN);

    char wifi_pass[WIFI_AP_PASS_LEN + 1] = {0};
    memcpy(wifi_pass,
           string_pico_serial + (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES) - WIFI_AP_SSID_LEN - WIFI_AP_PASS_LEN,
           WIFI_AP_PASS_LEN);
    strncpy(curr_pass, wifi_pass, WIFI_MAX_PASS_LEN);

    debug("disable sta");
    cyw43_arch_disable_sta_mode();

    debug("enable ap '%s' '%s'", wifi_ssid, wifi_pass);
    cyw43_arch_enable_ap_mode(wifi_ssid, wifi_pass, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t gw, mask;
    IP4_ADDR(&gw, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    debug("enable dhcp");
    dhcp_server_init(&dhcp_server, &gw, &mask);

    state = WS_READY;
    enabled_ap = true;

    cyw43_thread_exit();
}

static void wifi_connect(const char *ssid, const char *pw, uint32_t auth) {
    cyw43_thread_enter();

    debug("connecting to '%s'", ssid);
    strncpy(curr_ssid, ssid, WIFI_MAX_NAME_LEN);
    strncpy(curr_pass, pw, WIFI_MAX_PASS_LEN);

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
        state = WS_SCAN;
    } else {
        start_connect_time = to_ms_since_boot(get_absolute_time());
        state = WS_CONNECT;
    }

    cyw43_thread_exit();
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
    state = WS_SCAN;

    cyw43_wifi_scan_options_t scan_options = {0};
    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
    if (err != 0) {
        debug("error %d", err);
    }
}

void wifi_init(void) {
    if (state != WS_IDLE) {
        debug("invalid state %d", state);
        return;
    }

    cyw43_thread_enter();

    cyw43_arch_enable_sta_mode();

    wifi_scan();
    start_scan_time = to_ms_since_boot(get_absolute_time());

    cyw43_thread_exit();
}

void wifi_deinit(void) {
    cyw43_thread_enter();

    cyw43_arch_disable_sta_mode();
    cyw43_arch_disable_ap_mode();
    state = WS_IDLE;

    if (enabled_ap) {
        enabled_ap = false;
        dhcp_server_deinit(&dhcp_server);
    }

    cyw43_thread_exit();
}

bool wifi_initialized(void) {
    return (state != WS_IDLE);
}

bool wifi_ready(void) {
    return (state == WS_READY);
}

const char *wifi_state(void) {
    switch (state) {
    case WS_IDLE:
        return "Disabled";

    case WS_SCAN:
        return "Scanning";

    case WS_CONNECT:
        return "Connecting";

    case WS_WAIT_FOR_IP:
        return "Waiting for IP";

    case WS_READY: {
        uint32_t now = to_ms_since_boot(get_absolute_time()) % (enabled_ap ? 9000 : 6000);
        if (now < 3000) {
            // show SSID
            return curr_ssid;
        } else if (now < 6000) {
            // show IP
            cyw43_arch_lwip_begin();
            const ip4_addr_t *ip = netif_ip4_addr(netif_default);
            cyw43_arch_lwip_end();

            return ip4addr_ntoa(ip);
        } else {
            // show Pass (only AP)
            return curr_pass;
        }
    }
    }

    return NULL;
}

static err_t wifi_tcp_connect(void *arg, struct tcp_pcb *tpcb, err_t err) {
    (void)err;
    int c = (int)arg;

    err_t e = tcp_write(tpcb, cache[c].data, cache[c].len, TCP_WRITE_FLAG_COPY);
    if (e) {
        debug("tcp_write error %d", e);
        cache[c].clear = true;
        return ERR_OK;
    }

    e = tcp_output(tpcb);
    if (e) {
        debug("tcp_output error %d", e);
        cache[c].clear = true;
        return ERR_OK;
    }

    return ERR_OK;
}

static void wifi_tcp_error(void *arg, err_t err) {
    int c = (int)arg;
    debug("tcp err %d", err);
    cache[c].in_use = false; // lwip already freed the tpcb
}

static err_t wifi_tcp_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    (void)tpcb;
    (void)err;
    int c = (int)arg;
    if (p == NULL) {
        debug("remote closed conn");
        cache[c].clear = true;
    } else {
        debug("tcp rx %d: %s", pbuf_clen(p), (char *)p->payload);
        cache[c].clear = true;
    }
    return ERR_OK;
}

static err_t wifi_tcp_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    (void)tpcb;
    int c = (int)arg;
    if (len == cache[c].len) {
        debug("tcp tx success");
        cache[c].clear = true;
    }
    return ERR_OK;
}

void wifi_tcp_send(const char *ip, uint16_t port, const char *packet, unsigned int len) {
    cyw43_arch_lwip_begin();

    ip4_addr_t val;
    if (!ip4addr_aton(ip, &val)) {
        debug("invalid IP: %s", ip);
        cyw43_arch_lwip_end();
        return;
    }

    int c = -1;
    for (unsigned int i = 0; i < (sizeof(cache) / sizeof(cache[0])); i++) {
        if (!cache[i].in_use) {
            c = i;
        }
    }
    if (c < 0) {
        debug("no space in cache");
        cyw43_arch_lwip_end();
        return;
    }

    cache[c].in_use = true;
    cache[c].clear = false;
    cache[c].ip = val;
    cache[c].port = port;
    cache[c].tpcb = tcp_new();

    if (len > sizeof(cache[c].data)) {
        len = sizeof(cache[c].data);
    }

    cache[c].len = len;
    memcpy(cache[c].data, packet, len);

    tcp_arg(cache[c].tpcb, (void *)c);
    tcp_err(cache[c].tpcb, wifi_tcp_error);
    tcp_recv(cache[c].tpcb, wifi_tcp_recv);
    tcp_sent(cache[c].tpcb, wifi_tcp_sent);
    tcp_connect(cache[c].tpcb, &val, port, wifi_tcp_connect);

    cyw43_arch_lwip_end();
}

void wifi_run(void) {
    cyw43_thread_enter();

    if (state == WS_SCAN) {
        if (!cyw43_wifi_scan_active(&cyw43_state)) {
            debug("restarting scan");
            wifi_scan();
        }

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_scan_time) >= SCAN_AP_TIMEOUT_MS) {
            debug("wifi sta timeout. opening ap.");
            wifi_ap();
        }
    } else if (state == WS_CONNECT) {
        int link = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);

        static int prev_link = 0xFF;
        if (prev_link != link) {
            prev_link = link;
            debug("net link status: %d", link);
        }

        if (link == CYW43_LINK_JOIN) {
            debug("joined network");
            start_ip_time = to_ms_since_boot(get_absolute_time());
            state = WS_WAIT_FOR_IP;
        } else if (link < CYW43_LINK_DOWN) {
            debug("net connection failed. retry.");
            wifi_scan();
        }

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_connect_time) >= CONNECT_TIMEOUT_MS) {
            debug("net connection timeout. retry.");
            wifi_scan();
        }
    } else if (state == WS_WAIT_FOR_IP) {
        cyw43_arch_lwip_begin();
        const ip4_addr_t *ip = netif_ip4_addr(netif_default);
        cyw43_arch_lwip_end();

        if (ip4_addr_get_u32(ip) != 0) {
            state = WS_READY;
            debug("got IP '%s'", ip4addr_ntoa(ip));
        }

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - start_ip_time) >= CONNECT_TIMEOUT_MS) {
            debug("net dhcp timeout. retry.");

            //cyw43_arch_lwip_begin();
            //dhcp_renew(netif_default);
            //cyw43_arch_lwip_end();

            // DHCP renew does not seem to help, only a full reconnect
            wifi_scan();
        }
    }

    for (unsigned int c = 0; c < (sizeof(cache) / sizeof(cache[0])); c++) {
        if (cache[c].in_use && cache[c].clear) {
            tcp_close(cache[c].tpcb);
            cache[c].in_use = false;
        }
    }

    cyw43_thread_exit();
}
