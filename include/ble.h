/*
 * ble.h
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

#ifndef __BLE_H__
#define __BLE_H__

#include "btstack.h"

#define BLE_MAX_NAME_LENGTH 32
#define BLE_MAX_SCAN_RESULTS 32

enum ble_scan_mode {
    BLE_SCAN_OFF    = 0,
    BLE_SCAN_ON     = 1,
    BLE_SCAN_TOGGLE = 2,
};

enum ble_remote_name_state {
    BLE_NAME_REQUEST = 0,
    BLE_NAME_INQUIRED,
    BLE_NAME_FETCHED,
};

struct ble_scan_result {
    bool set;
    uint32_t time;
    enum ble_remote_name_state state;

    bd_addr_t addr;
    bd_addr_type_t type;
    int8_t rssi;
    uint8_t page_scan_repetition_mode;
    uint16_t clock_offset;
    uint32_t class;

    char name[BLE_MAX_NAME_LENGTH + 1];
};

void ble_init(void);
void ble_scan(enum ble_scan_mode mode);
int ble_get_scan_results(struct ble_scan_result *buf, uint len);

#endif // __BLE_H__
