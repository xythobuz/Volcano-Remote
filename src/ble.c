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

enum ble_state {
    TC_OFF = 0,
    TC_IDLE,
    TC_W4_SCAN_RESULT,
    TC_W4_CONNECT,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_RESULT,
    TC_W4_ENABLE_NOTIFICATIONS_COMPLETE,
    TC_W4_READY
};

static btstack_packet_callback_registration_t hci_event_callback_registration;
static enum ble_state state = TC_OFF;
static struct ble_scan_result scans[BLE_MAX_SCAN_RESULTS] = {0};

// TODO scan result entries are not aging out

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
            // already in list, just update time for aging
            scans[i].time = to_ms_since_boot(get_absolute_time());
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

    if (packet_type != HCI_EVENT_PACKET) {
        debug("unexpected packet 0x%02X", packet_type);
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

    case HCI_EVENT_INQUIRY_COMPLETE:
    case HCI_EVENT_COMMAND_STATUS:
    case HCI_EVENT_REMOTE_HOST_SUPPORTED_FEATURES:
    case BTSTACK_EVENT_SCAN_MODE_CHANGED:
    case HCI_EVENT_COMMAND_COMPLETE:
    case HCI_EVENT_TRANSPORT_PACKET_SENT:
        break;

    case GAP_EVENT_ADVERTISING_REPORT: {
        if (state != TC_W4_SCAN_RESULT) {
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

            case BLUETOOTH_DATA_TYPE_FLAGS:
            case BLUETOOTH_DATA_TYPE_TX_POWER_LEVEL:
            case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
            case BLUETOOTH_DATA_TYPE_SERVICE_DATA_16_BIT_UUID:
            case BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA:
            case BLUETOOTH_DATA_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE:
                break;

            default:
                debug("Unexpected advertisement type 0x%02X from %s", data_type, bd_addr_to_str(addr));
                hexdump(data, data_size);
                break;
            }
        }
        break;
    }

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ADVERTISING_REPORT:
                // handled internally by BTstack
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                debug("connection complete?!");
                break;

            default:
                debug("unexpected LE meta event 0x%02X", hci_event_le_meta_get_subevent_code(packet));
                break;
        }
        break;

    default:
        debug("unexpected event 0x%02X", hci_event_packet_get_type(packet));
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

    switch (mode) {
    case BLE_SCAN_OFF:
        debug("stopping BLE scan");
        gap_stop_scan();
        state = TC_IDLE;
        break;

    case BLE_SCAN_ON:
        debug("starting BLE scan");
        state = TC_W4_SCAN_RESULT;
        gap_set_scan_parameters(1, 0x0030, 0x0030);
        gap_start_scan();
        break;

    case BLE_SCAN_TOGGLE:
        switch (state) {
        case TC_W4_SCAN_RESULT:
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

int ble_get_scan_results(struct ble_scan_result *buf, uint len) {
    if (!buf || (len <= 0)) {
        return -1;
    }

    cyw43_thread_enter();

    uint pos = 0;
    for (uint i = 0; i < BLE_MAX_SCAN_RESULTS; i++) {
        if (!scans[i].set) {
            continue;
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
