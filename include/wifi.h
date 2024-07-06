/*
 * wifi.h
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

#ifndef __WIFI_H__
#define __WIFI_H__

#include <stdbool.h>

#define WIFI_MAX_NET_COUNT 5
#define WIFI_MAX_NAME_LEN 32
#define WIFI_MAX_PASS_LEN 32

struct net_credentials {
    char name[WIFI_MAX_NAME_LEN];
    char pass[WIFI_MAX_PASS_LEN];
};

void wifi_init(void);
void wifi_deinit(void);

bool wifi_initialized(void);
bool wifi_ready(void);
const char *wifi_state(void);

void wifi_run(void);

void wifi_tcp_send(const char *ip, uint16_t port, const char *packet, unsigned int len);

#endif // __WIFI_H__
