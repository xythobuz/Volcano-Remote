/*
 * ble.c
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/pico_w/bt/standalone/client.c
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

#include "btstack.h"
#include "pico/cyw43_arch.h"

#include "config.h"
#include "log.h"
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

static bd_addr_t server_addr;
static bd_addr_type_t server_addr_type;

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

    case HCI_EVENT_COMMAND_COMPLETE:
    case HCI_EVENT_TRANSPORT_PACKET_SENT:
        break;

    case GAP_EVENT_ADVERTISING_REPORT:
        if (state != TC_W4_SCAN_RESULT) {
            debug("scan result in invalid state %d", state);
            return;
        }

        gap_event_advertising_report_get_address(packet, server_addr);
        server_addr_type = gap_event_advertising_report_get_address_type(packet);
        debug("Found device with addr %s", bd_addr_to_str(server_addr));
        break;

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ADVERTISING_REPORT:
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
    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    gatt_client_init();

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    hci_power_control(HCI_POWER_ON);
}

void ble_scan(enum ble_scan_mode mode) {
    switch (mode) {
    case BLE_SCAN_OFF:
        debug("stopping BLE scan");
        state = TC_IDLE;
        gap_stop_scan();
        break;

    case BLE_SCAN_ON:
        debug("starting BLE scan");
        state = TC_W4_SCAN_RESULT;
        gap_set_scan_parameters(0,0x0030, 0x0030);
        gap_start_scan();
        break;

    case BLE_SCAN_TOGGLE:
        switch (state) {
        case TC_W4_SCAN_RESULT:
            ble_scan(0);
            break;

        case TC_IDLE:
            ble_scan(1);
            break;

        default:
            debug("invalid state %d", state);
            break;
        }
        break;

    default:
        debug("invalid mode %d", mode);
        break;
    }
}
