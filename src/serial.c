/*
 * serial.c
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

// UART0, Tx GP0, Rx GP1
#define UART_ID uart0
#define UART_IRQ UART0_IRQ
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Serial port parameters
#define BAUD_RATE 115200
#define DATA_BITS 8
#define PARITY UART_PARITY_NONE
#define STOP_BITS 1

#define UART_HW_FIFO_LEN 32
#define UART_TX_BUFF_LEN 128

#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "config.h"
#include "console.h"
#include "log.h"
#include "util.h"
#include "ring.h"
#include "serial.h"

static uint8_t tx_buff[UART_TX_BUFF_LEN] = {0};
static struct ring_buffer tx = RB_INIT(tx_buff, sizeof(tx_buff));
static bool reroute_serial_debug = false;

static void serial_irq(void) {
    uint8_t buf[UART_HW_FIFO_LEN];
    uint16_t count = 0;

    // Rx - read from UART FIFO to local buffer
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        buf[count++] = ch;

        if (count >= UART_HW_FIFO_LEN) {
            break;
        }
    }

    // Rx - pass local buffer to further processing
    if ((count >= 1) && (buf[0] == ENTER_BOOTLOADER_MAGIC)) {
        reset_to_bootloader();
    } else if (reroute_serial_debug) {
        debug_handle_input((char *)buf, count);
    } else {
        cnsl_handle_input((char *)buf, count);
    }

    // Tx - write to UART FIFO if needed
    while (uart_is_writable(UART_ID) && (rb_len(&tx) > 0)) {
        uart_putc_raw(UART_ID, rb_pop(&tx));
    }
}

void serial_init(void) {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);

    irq_set_exclusive_handler(UART_IRQ, serial_irq);
    irq_set_enabled(UART_IRQ, true);

    uart_set_irq_enables(UART_ID, true, false);
}

void serial_write(const uint8_t *buf, size_t count) {
    rb_add(&tx, buf, count);
}

void serial_set_reroute(bool reroute) {
    reroute_serial_debug = reroute;
}
