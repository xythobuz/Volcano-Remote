/*
 * serial.c
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/uart/uart_advanced/uart_advanced.c
 * https://forums.raspberrypi.com/viewtopic.php?t=343110
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

#define UART_RX_BUFF_LEN 64
#define UART_TX_BUFF_LEN 128

#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "config.h"
#include "console.h"
#include "log.h"
#include "util.h"
#include "ring.h"
#include "serial.h"

static uint8_t rx_buff[UART_RX_BUFF_LEN] = {0};
static struct ring_buffer rx = RB_INIT(rx_buff, sizeof(rx_buff));

static uint8_t tx_buff[UART_TX_BUFF_LEN] = {0};
static struct ring_buffer tx = RB_INIT(tx_buff, sizeof(tx_buff));

static bool reroute_serial_debug = false;
static bool tx_irq_state = false;
static bool rx_irq_state = false;
#define SET_TX_IRQ(v) {                 \
    tx_irq_state = v;                   \
    uart_set_irq_enables(UART_ID,       \
                         rx_irq_state,  \
                         tx_irq_state); \
}
#define SET_RX_IRQ(v) {                 \
    rx_irq_state = v;                   \
    uart_set_irq_enables(UART_ID,       \
                         rx_irq_state,  \
                         tx_irq_state); \
}

static void serial_irq(void) {
    // Rx - read from UART FIFO to local buffer
    while (uart_is_readable(UART_ID) && (rb_space(&rx) > 0)) {
        uint8_t ch = uart_getc(UART_ID);
        rb_push(&rx, ch);
    }

    // Tx - write to UART FIFO if needed
    while (uart_is_writable(UART_ID)) {
        if (rb_len(&tx) > 0) {
            uart_putc_raw(UART_ID, rb_pop(&tx));
        } else {
            SET_TX_IRQ(false);
            break;
        }
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

    SET_RX_IRQ(true);
    SET_TX_IRQ(false);
}

void serial_write(const uint8_t *buf, size_t count) {
    SET_TX_IRQ(false);

    while ((rb_len(&tx) == 0) && uart_is_writable(UART_ID) && (count > 0)) {
        uart_putc_raw(UART_ID, *buf++);
        count--;
    }

    if (count == 0) {
        return;
    }

    size_t off = 0;

#ifdef SERIAL_WRITES_BLOCK_WHEN_BUFFER_FULL
    while (count > rb_space(&tx)) {
        size_t space = rb_space(&tx);
        rb_add(&tx, buf + off, space);
        count -= space;
        off += space;

        SET_TX_IRQ(true);

        sleep_ms(1);

        SET_TX_IRQ(false);
    }
#endif // SERIAL_WRITES_BLOCK_WHEN_BUFFER_FULL

    rb_add(&tx, buf + off, count);

    SET_TX_IRQ(true);
}

void serial_set_reroute(bool reroute) {
    reroute_serial_debug = reroute;
}

void serial_run(void) {
    SET_RX_IRQ(false);

    if (rb_len(&rx) >= 1) {
        if (rb_peek(&rx) == ENTER_BOOTLOADER_MAGIC) {
            reset_to_bootloader();
        } else if (reroute_serial_debug) {
            rb_move(&rx, debug_handle_input);
        } else {
            rb_move(&rx, cnsl_handle_input);
        }
    }

    SET_RX_IRQ(true);
}
