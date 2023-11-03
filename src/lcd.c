/*
 * lcd.c
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

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "driver_st7789.h"

#include "config.h"
#include "log.h"
#include "lcd.h"

#define LCD_PIN_DC 8
#define LCD_PIN_CS 9
#define LCD_PIN_CLK 10
#define LCD_PIN_DIN 11
#define LCD_PIN_RST 12
#define LCD_PIN_BL 13

#define ST7789_PICO_COLUMN                             240
#define ST7789_PICO_ROW                                240

#define ST7789_PICO_ACCESS                            (ST7789_ORDER_PAGE_TOP_TO_BOTTOM | \
                                                       ST7789_ORDER_COLUMN_LEFT_TO_RIGHT | \
                                                       ST7789_ORDER_PAGE_COLUMN_NORMAL | \
                                                       ST7789_ORDER_LINE_TOP_TO_BOTTOM | \
                                                       ST7789_ORDER_COLOR_RGB | \
                                                       ST7789_ORDER_REFRESH_LEFT_TO_RIGHT)

#define ST7789_PICO_RGB_INTERFACE_COLOR_FORMAT         0
#define ST7789_PICO_CONTROL_INTERFACE_COLOR_FORMAT     ST7789_CONTROL_INTERFACE_COLOR_FORMAT_16_BIT

#define ST7789_PICO_PORCH_NORMAL_BACK                  0x0C
#define ST7789_PICO_PORCH_NORMAL_FRONT                 0x0C
#define ST7789_PICO_PORCH_ENABLE                       ST7789_BOOL_FALSE
#define ST7789_PICO_PORCH_IDEL_BACK                    0x03
#define ST7789_PICO_PORCH_IDEL_FRONT                   0x03
#define ST7789_PICO_PORCH_PART_BACK                    0x03
#define ST7789_PICO_PORCH_PART_FRONT                   0x03

#define ST7789_PICO_VGHS                               ST7789_VGHS_13P26_V
#define ST7789_PICO_VGLS_NEGATIVE                      ST7789_VGLS_NEGATIVE_10P43

#define ST7789_PICO_VCOMS                              0.725f

#define ST7789_PICO_XMY                                ST7789_BOOL_FALSE
#define ST7789_PICO_XBGR                               ST7789_BOOL_TRUE
#define ST7789_PICO_XINV                               ST7789_BOOL_FALSE
#define ST7789_PICO_XMX                                ST7789_BOOL_TRUE
#define ST7789_PICO_XMH                                ST7789_BOOL_TRUE
#define ST7789_PICO_XMV                                ST7789_BOOL_FALSE
#define ST7789_PICO_XGS                                ST7789_BOOL_FALSE

#define ST7789_PICO_VDV_VRH_FROM                       ST7789_VDV_VRH_FROM_CMD
#define ST7789_PICO_VRHS                               4.45f
#define ST7789_PICO_VDV                                0.0f

#define ST7789_PICO_INVERSION_SELECTION                ST7789_INVERSION_SELECTION_DOT
#define ST7789_PICO_FRAME_RATE                         ST7789_FRAME_RATE_60_HZ

#define ST7789_PICO_AVDD                               ST7789_AVDD_6P8_V
#define ST7789_PICO_AVCL_NEGTIVE                       ST7789_AVCL_NEGTIVE_4P8_V
#define ST7789_PICO_VDS                                ST7789_VDS_2P3_V

#define ST7789_PICO_POSITIVE_VOLTAGE_GAMMA            {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, \
                                                       0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}
#define ST7789_PICO_NEGATIVA_VOLTAGE_GAMMA            {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, \
                                                       0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}

static st7789_handle_t gs_handle;
static uint16_t bl_value = 0;

static uint8_t st7789_interface_spi_init(void) {
    // Use SPI1 at 100MHz
    spi_init(spi1, 100 * 1000 * 1000);
    gpio_set_function(LCD_PIN_CLK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_PIN_DIN, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(LCD_PIN_CS);
    gpio_set_dir(LCD_PIN_CS, GPIO_OUT);
    gpio_put(LCD_PIN_CS, 1);

    spi_set_format(spi1,
                   8, // Number of bits per transfer
                   0, // Polarity (CPOL)
                   0, // Phase (CPHA)
                   SPI_MSB_FIRST);

    return 0;
}

static uint8_t st7789_interface_spi_deinit(void) {
    spi_deinit(spi1);
    gpio_deinit(LCD_PIN_CS);
    return 0;
}

static uint8_t st7789_interface_spi_write_cmd(uint8_t *buf, uint16_t len) {
    gpio_put(LCD_PIN_CS, 0);
    spi_write_blocking(spi1, buf, len);
    gpio_put(LCD_PIN_CS, 1);
    return 0;
}

static void st7789_interface_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

static void st7789_interface_debug_print(const char *const fmt, ...) {
    va_list args;
    va_start(args, fmt);
    debug_log_va(true, fmt, args);
    va_end(args);
}

static uint8_t st7789_interface_cmd_data_gpio_init(void) {
    gpio_init(LCD_PIN_DC);
    gpio_set_dir(LCD_PIN_DC, GPIO_OUT);
    return 0;
}

static uint8_t st7789_interface_cmd_data_gpio_deinit(void) {
    gpio_deinit(LCD_PIN_DC);
    return 0;
}

static uint8_t st7789_interface_cmd_data_gpio_write(uint8_t value) {
    gpio_put(LCD_PIN_DC, value);
    return 0;
}

static uint8_t st7789_interface_reset_gpio_init(void) {
    gpio_init(LCD_PIN_RST);
    gpio_set_dir(LCD_PIN_RST, GPIO_OUT);
    return 0;
}

static uint8_t st7789_interface_reset_gpio_deinit(void) {
    gpio_deinit(LCD_PIN_RST);
    return 0;
}

static uint8_t st7789_interface_reset_gpio_write(uint8_t value) {
    gpio_put(LCD_PIN_RST, value);
    return 0;
}

void lcd_init(void) {
    uint8_t reg;

    DRIVER_ST7789_LINK_INIT(&gs_handle, st7789_handle_t);
    DRIVER_ST7789_LINK_SPI_INIT(&gs_handle, st7789_interface_spi_init);
    DRIVER_ST7789_LINK_SPI_DEINIT(&gs_handle, st7789_interface_spi_deinit);
    DRIVER_ST7789_LINK_SPI_WRITE_COMMAND(&gs_handle, st7789_interface_spi_write_cmd);
    DRIVER_ST7789_LINK_COMMAND_DATA_GPIO_INIT(&gs_handle, st7789_interface_cmd_data_gpio_init);
    DRIVER_ST7789_LINK_COMMAND_DATA_GPIO_DEINIT(&gs_handle, st7789_interface_cmd_data_gpio_deinit);
    DRIVER_ST7789_LINK_COMMAND_DATA_GPIO_WRITE(&gs_handle, st7789_interface_cmd_data_gpio_write);
    DRIVER_ST7789_LINK_RESET_GPIO_INIT(&gs_handle, st7789_interface_reset_gpio_init);
    DRIVER_ST7789_LINK_RESET_GPIO_DEINIT(&gs_handle, st7789_interface_reset_gpio_deinit);
    DRIVER_ST7789_LINK_RESET_GPIO_WRITE(&gs_handle, st7789_interface_reset_gpio_write);
    DRIVER_ST7789_LINK_DELAY_MS(&gs_handle, st7789_interface_delay_ms);
    DRIVER_ST7789_LINK_DEBUG_PRINT(&gs_handle, st7789_interface_debug_print);

    st7789_init(&gs_handle);

    st7789_set_column(&gs_handle, ST7789_PICO_COLUMN);

    st7789_set_row(&gs_handle, ST7789_PICO_ROW);

    st7789_set_memory_data_access_control(&gs_handle, ST7789_PICO_ACCESS);

    st7789_set_interface_pixel_format(&gs_handle,
                                      ST7789_PICO_RGB_INTERFACE_COLOR_FORMAT,
                                      ST7789_PICO_CONTROL_INTERFACE_COLOR_FORMAT);

    st7789_set_porch(&gs_handle,
                     ST7789_PICO_PORCH_NORMAL_BACK,
                     ST7789_PICO_PORCH_NORMAL_FRONT,
                     ST7789_PICO_PORCH_ENABLE,
                     ST7789_PICO_PORCH_IDEL_BACK,
                     ST7789_PICO_PORCH_IDEL_FRONT,
                     ST7789_PICO_PORCH_PART_BACK,
                     ST7789_PICO_PORCH_PART_FRONT);

    st7789_set_gate_control(&gs_handle, ST7789_PICO_VGHS, ST7789_PICO_VGLS_NEGATIVE);

    st7789_vcom_convert_to_register(&gs_handle, ST7789_PICO_VCOMS, &reg);
    st7789_set_vcoms(&gs_handle, reg);

    st7789_set_lcm_control(&gs_handle,
                           ST7789_PICO_XMY,
                           ST7789_PICO_XBGR,
                           ST7789_PICO_XINV,
                           ST7789_PICO_XMX,
                           ST7789_PICO_XMH,
                           ST7789_PICO_XMV,
                           ST7789_PICO_XGS);

    st7789_set_vdv_vrh_from(&gs_handle, ST7789_PICO_VDV_VRH_FROM);

    st7789_vrhs_convert_to_register(&gs_handle, ST7789_PICO_VRHS, &reg);
    st7789_set_vrhs(&gs_handle, reg);

    st7789_vdv_convert_to_register(&gs_handle, ST7789_PICO_VDV, &reg);
    st7789_set_vdv(&gs_handle, reg);

    st7789_set_frame_rate(&gs_handle, ST7789_PICO_INVERSION_SELECTION, ST7789_PICO_FRAME_RATE);

    st7789_set_power_control_1(&gs_handle,
                               ST7789_PICO_AVDD,
                               ST7789_PICO_AVCL_NEGTIVE,
                               ST7789_PICO_VDS);

    uint8_t param_positive[14] = ST7789_PICO_POSITIVE_VOLTAGE_GAMMA;
    st7789_set_positive_voltage_gamma_control(&gs_handle, param_positive);

    uint8_t param_negative[14] = ST7789_PICO_NEGATIVA_VOLTAGE_GAMMA;
    st7789_set_negative_voltage_gamma_control(&gs_handle, param_negative);

    st7789_display_inversion_on(&gs_handle);

    st7789_sleep_out(&gs_handle);

    st7789_display_on(&gs_handle);

    st7789_clear(&gs_handle);

    // backlight
    uint bl_slice = pwm_gpio_to_slice_num(LCD_PIN_BL);
    uint bl_channel = pwm_gpio_to_channel(LCD_PIN_BL);
    gpio_set_function(LCD_PIN_BL, GPIO_FUNC_PWM);
    pwm_set_wrap(bl_slice, 0xFFFF);
    pwm_set_clkdiv(bl_slice, 1.0f);
    pwm_set_chan_level(bl_slice, bl_channel, bl_value);
    pwm_set_enabled(bl_slice, true);
}

uint16_t lcd_get_backlight(void) {
    return bl_value;
}

void lcd_set_backlight(uint16_t value) {
    uint bl_slice = pwm_gpio_to_slice_num(LCD_PIN_BL);
    uint bl_channel = pwm_gpio_to_channel(LCD_PIN_BL);
    bl_value = value;
    pwm_set_chan_level(bl_slice, bl_channel, bl_value);
}

void lcd_write_point(uint16_t x, uint16_t y, uint32_t color) {
    st7789_draw_point(&gs_handle, x, y, color);

    st7789_display_on(&gs_handle);
}
