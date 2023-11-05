/*
 * ble.c
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/pico_w/bt/standalone/client.c
 * https://vanhunteradams.com/Pico/BLE/BTStack_HCI.html
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

#include "config.h"
#include "log.h"
#include "util.h"
#include "ble.h"

#define BLE_MAX_SCAN_AGE_MS (10 * 1000)

enum ble_state {
    TC_OFF = 0,
    TC_IDLE,
    TC_W4_SCAN,
    TC_W4_CONNECT,
    TC_READY,
    TC_W4_READ,
    TC_READ_COMPLETE,
};

static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t connection_handle;

static enum ble_state state = TC_OFF;
static struct ble_scan_result scans[BLE_MAX_SCAN_RESULTS] = {0};
static uint16_t read_len = 0;
static uint8_t read_buff[BLE_MAX_VALUE_LEN] = {0};

static void hci_add_scan_result(bd_addr_t addr, bd_addr_type_t type, int8_t rssi) {
    int unused = -1;

    for (uint i = 0; i < BLE_MAX_SCAN_RESULTS; i++) {
        if (!scans[i].set) {
            if (unused < 0) {
                unused = i;
            }
            continue;
        }

        if (memcmp(addr, scans[i].addr, sizeof(bd_addr_t)) == 0) {
            // already in list, just update changing values
            scans[i].time = to_ms_since_boot(get_absolute_time());
            scans[i].rssi = rssi;
            return;
        }
    }

    if (unused < 0) {
        debug("no space in scan results for %s", bd_addr_to_str(addr));
        return;
    }

    debug("new device with addr %s", bd_addr_to_str(addr));
    scans[unused].set = true;
    scans[unused].time = to_ms_since_boot(get_absolute_time());
    memcpy(scans[unused].addr, addr, sizeof(bd_addr_t));
    scans[unused].type = type;
    scans[unused].rssi = rssi;
    scans[unused].name[0] = '\0';
}

static void hci_scan_result_add_name(bd_addr_t addr, const uint8_t *data, uint8_t data_size) {
    for (uint i = 0; i < BLE_MAX_SCAN_RESULTS; i++) {
        if (!scans[i].set) {
            continue;
        }
        if (memcmp(addr, scans[i].addr, sizeof(bd_addr_t)) != 0) {
            continue;
        }

        uint8_t len = data_size;
        if (len > BLE_MAX_NAME_LENGTH) {
            len = BLE_MAX_NAME_LENGTH;
        }
        memcpy(scans[i].name, data, len);
        scans[i].name[len] = '\0';
        scans[i].time = to_ms_since_boot(get_absolute_time());
        return;
    }

    debug("no matching entry for %s to add name '%.*s' to", bd_addr_to_str(addr), data_size, data);
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);
    UNUSED(channel);

    //debug("type=0x%02X size=%d", packet_type, size);
    //hexdump(packet, size);

    if (packet_type != HCI_EVENT_PACKET) {
        //debug("unexpected packet 0x%02X", packet_type);
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                bd_addr_t local_addr;
                gap_local_bd_addr(local_addr);
                debug("BTstack up with %s", bd_addr_to_str(local_addr));
                state = TC_IDLE;
            } else {
                debug("BTstack down (%d)", btstack_event_state_get_state(packet));
                state = TC_OFF;
            }
        break;

    case GAP_EVENT_ADVERTISING_REPORT: {
        if (state != TC_W4_SCAN) {
            debug("scan result in invalid state %d", state);
            return;
        }

        bd_addr_t addr;
        gap_event_advertising_report_get_address(packet, addr);

        bd_addr_type_t type;
        type = gap_event_advertising_report_get_address_type(packet);

        int8_t rssi;
        rssi = (int8_t)gap_event_advertising_report_get_rssi(packet);

        // add data received so far
        hci_add_scan_result(addr, type, rssi);

        // get advertisement from report event
        const uint8_t *adv_data = gap_event_advertising_report_get_data(packet);
        uint8_t adv_len = gap_event_advertising_report_get_data_length(packet);

        // iterate over advertisement data
        ad_context_t context;
        for (ad_iterator_init(&context, adv_len, adv_data);
             ad_iterator_has_more(&context);
             ad_iterator_next(&context)) {
            uint8_t data_type = ad_iterator_get_data_type(&context);
            uint8_t data_size = ad_iterator_get_data_len(&context);
            const uint8_t *data = ad_iterator_get_data(&context);
            switch (data_type) {
            case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
            case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
                hci_scan_result_add_name(addr, data, data_size);
                break;

            default:
                //debug("Unexpected advertisement type 0x%02X from %s", data_type, bd_addr_to_str(addr));
                //hexdump(data, data_size);
                break;
            }
        }
        break;
    }

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                if (state != TC_W4_CONNECT) {
                    return;
                }
                debug("connection complete");
                connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                state = TC_READY;
                break;

            default:
                //debug("unexpected LE meta event 0x%02X", hci_event_le_meta_get_subevent_code(packet));
                break;
        }
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        debug("disconnected");
        connection_handle = HCI_CON_HANDLE_INVALID;
        state = TC_IDLE;
        break;

    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
        if (state != TC_W4_READ) {
            debug("gatt query result in invalid state %d", state);
            return;
        }
        uint16_t len = gatt_event_characteristic_value_query_result_get_value_length(packet);
        if ((read_len + len) > BLE_MAX_VALUE_LEN) {
            debug("not enough space for value (%d + %d > %d)", read_len, len, BLE_MAX_VALUE_LEN);
            return;
        }
        memcpy(read_buff + read_len,
               gatt_event_characteristic_value_query_result_get_value(packet),
               len);
        read_len += len;
        break;

    case GATT_EVENT_QUERY_COMPLETE:
        if (state != TC_W4_READ) {
            debug("gatt query complete in invalid state %d", state);
            return;
        }
        state = TC_READ_COMPLETE;
        break;

    default:
        //debug("unexpected event 0x%02X", hci_event_packet_get_type(packet));
        break;
    }
}

void ble_init(void) {
    cyw43_thread_enter();

    for (uint i = 0; i < BLE_MAX_SCAN_RESULTS; i++) {
        scans[i].set = false;
    }
    state = TC_OFF;

    cyw43_thread_exit();

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    gatt_client_init();

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    hci_power_control(HCI_POWER_ON);
}

bool ble_is_ready(void) {
    cyw43_thread_enter();

    bool v = (state != TC_OFF);

    cyw43_thread_exit();
    return v;
}

void ble_scan(enum ble_scan_mode mode) {
    cyw43_thread_enter();

    if (state == TC_OFF) {
        cyw43_thread_exit();
        return;
    }

    switch (mode) {
    case BLE_SCAN_OFF:
        debug("stopping BLE scan");
        gap_stop_scan();
        state = TC_IDLE;
        break;

    case BLE_SCAN_ON:
        debug("starting BLE scan");
        state = TC_W4_SCAN;
        gap_set_scan_parameters(1, 0x0030, 0x0030);
        gap_start_scan();
        break;

    case BLE_SCAN_TOGGLE:
        switch (state) {
        case TC_W4_SCAN:
            cyw43_thread_exit();
            ble_scan(0);
            return;

        case TC_IDLE:
            cyw43_thread_exit();
            ble_scan(1);
            return;

        default:
            debug("invalid state %d", state);
            break;
        }
        break;

    default:
        debug("invalid mode %d", mode);
        break;
    }

    cyw43_thread_exit();
}

int32_t ble_get_scan_results(struct ble_scan_result *buf, uint16_t len) {
    if (!buf || (len <= 0)) {
        return -1;
    }

    cyw43_thread_enter();

    if (state == TC_OFF) {
        cyw43_thread_exit();
        return -1;
    }

    uint16_t pos = 0;
    for (uint16_t i = 0; i < BLE_MAX_SCAN_RESULTS; i++) {
        if (!scans[i].set) {
            continue;
        }

        uint32_t diff = to_ms_since_boot(get_absolute_time()) - scans[i].time;
        if (diff >= BLE_MAX_SCAN_AGE_MS) {
            //debug("removing %s due to age", bd_addr_to_str(scans[i].addr));
            scans[i].set = false;
        }

        memcpy(buf + pos, scans + i, sizeof(struct ble_scan_result));
        pos++;

        if (pos >= len) {
            break;
        }
    }

    cyw43_thread_exit();
    return pos;
}

void ble_connect(bd_addr_t addr, bd_addr_type_t type) {
    cyw43_thread_enter();

    switch (state) {
    case TC_OFF:
        cyw43_thread_exit();
        return;

    case TC_W4_SCAN:
        cyw43_thread_exit();
        ble_scan(0);
        cyw43_thread_enter();
        break;

    case TC_READY:
        gap_disconnect(connection_handle);
        break;

    default:
        break;
    }

    debug("connecting to %s", bd_addr_to_str(addr));
    state = TC_W4_CONNECT;
    gap_connect(addr, type);

    cyw43_thread_exit();
}

bool ble_is_connected(void) {
    cyw43_thread_enter();

    bool v = (state == TC_READY)
             || (state == TC_W4_READ)
             || (state == TC_READ_COMPLETE);

    cyw43_thread_exit();
    return v;
}

void ble_disconnect(void) {
    cyw43_thread_enter();

    if (state == TC_READY) {
        gap_disconnect(connection_handle);
    }

    cyw43_thread_exit();
}

int32_t ble_read(const uint8_t *uuid, uint8_t *buff, uint16_t buff_len) {
    cyw43_thread_enter();

    if (state != TC_READY) {
        cyw43_thread_exit();
        debug("invalid state for read (%d)", state);
        return -1;
    }

    uint8_t r = gatt_client_read_value_of_characteristics_by_uuid128(hci_event_handler,
                                                                     connection_handle,
                                                                     0x0001, 0xFFFF, uuid);
    if (r != ERROR_CODE_SUCCESS) {
        cyw43_thread_exit();
        debug("gatt read failed %d", r);
        return -2;
    }

    state = TC_W4_READ;
    read_len = 0;
    cyw43_thread_exit();

    while (1) {
        sleep_ms(1);

        // TODO timeout

        cyw43_thread_enter();
        enum ble_state state_cached = state;
        cyw43_thread_exit();

        if (state_cached == TC_READ_COMPLETE) {
            break;
        }
    }

    cyw43_thread_enter();

    state = TC_READY;

    if (read_len > buff_len) {
        debug("buffer too short (%d < %d)", buff_len, read_len);
        cyw43_thread_exit();
        return -3;
    }

    memcpy(buff, read_buff, read_len);

    cyw43_thread_exit();
    return read_len;
}

int32_t ble_write(const uint8_t *uuid, uint8_t *buff, uint16_t buff_len) {
    UNUSED(uuid);
    UNUSED(buff);
    UNUSED(buff_len);
    return -1;
}
