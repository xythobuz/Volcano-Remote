/*
 * console.c
 *
 * Copyright (c) 2022 - 2023 Thomas Buck (thomas@xythobuz.de)
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

#include <inttypes.h>
#include <string.h>
#include "pico/stdlib.h"
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "util.h"
#include "usb_cdc.h"
#include "usb_msc.h"
#include "debug.h"
#include "console.h"
#include "lipo.h"
#include "ble.h"
#include "text.h"
#include "lcd.h"
#include "image.h"

#define CNSL_BUFF_SIZE 1024
#define CNSL_REPEAT_MS 500

//#define CNSL_REPEAT_PMW_STATUS_BY_DEFAULT

static char cnsl_line_buff[CNSL_BUFF_SIZE + 1];
static uint32_t cnsl_buff_pos = 0;

static char cnsl_last_command[CNSL_BUFF_SIZE + 1];

static char cnsl_repeated_command[CNSL_BUFF_SIZE + 1];
static bool repeat_command = false;
static uint32_t last_repeat_time = 0;

static void cnsl_interpret(const char *line) {
    if (strlen(line) == 0) {
        if ((strlen(cnsl_last_command) > 0) && (strcmp(cnsl_last_command, "repeat") != 0)) {
            // repeat last command once
            println("repeating command \"%s\"", cnsl_last_command);
            cnsl_interpret(cnsl_last_command);
            println();
        }
        return;
    } else if (strcmp(line, "repeat") == 0) {
        if (!repeat_command) {
            // mark last command to be repeated multiple times
            strncpy(cnsl_repeated_command, cnsl_last_command, CNSL_BUFF_SIZE + 1);
            last_repeat_time = to_ms_since_boot(get_absolute_time()) - 1001;
            repeat_command = true;
        } else {
            // stop repeating
            repeat_command = false;
        }
    } else if ((strcmp(line, "help") == 0)
            || (strcmp(line, "h") == 0)
            || (strcmp(line, "?") == 0)) {
        println("VolcanoRC Firmware Usage:");
        println("  reset - reset back into this firmware");
        println("   \\x18 - reset to bootloader");
        println(" repeat - repeat last command every %d milliseconds", CNSL_REPEAT_MS);
        println("   help - print this message");
        println("  mount - make mass storage medium (un)available");
        println("  power - show Lipo battery status");
        println("   scan - start or stop BLE scan");
        println("scanres - print list of found BLE devices");
        println("  clear - blank screen");
        println(" splash - draw image on screen");
        println("  fonts - show font list");
        println("   text - draw text on screen");
        println("    bat - draw battery indicator");
        println("Press Enter with no input to repeat last command.");
        println("Use repeat to continuously execute last command.");
        println("Stop this by calling repeat again.");
    } else if (strcmp(line, "reset") == 0) {
        reset_to_main();
    } else if (strcmp(line, "mount") == 0) {
        bool state = msc_is_medium_available();
        println("Currently %s. %s now.",
                state ? "mounted" : "unmounted",
                state ? "Unplugging" : "Plugging in");
        msc_set_medium_available(!state);
    } else if (strcmp(line, "power") == 0) {
        float volt = lipo_voltage();
        println("Battery: %.2fV = %.1f%% @ %s",
                volt, lipo_percentage(volt),
                lipo_charging() ? "charging" : "draining");
    } else if (strcmp(line, "scan") == 0) {
        ble_scan(2);
    } else if (strcmp(line, "scanres") == 0) {
        struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
        int n = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);
        if (n < 0) {
            println("Error reading results (%d)", n);
        } else {
            println("%d results", n);
            for (int i = 0; i < n; i++) {
                uint32_t age = to_ms_since_boot(get_absolute_time()) - results[i].time;
                println("addr=%s type=%d rssi=%d age=%.1fs name='%s'",
                        bd_addr_to_str(results[i].addr),
                        results[i].type, results[i].rssi,
                        age / 1000.0, results[i].name);
            }
        }
    } else if (strcmp(line, "clear") == 0) {
        lcd_clear();
    } else if (strcmp(line, "splash") == 0) {
        draw_splash();
    } else if (strcmp(line, "fonts") == 0) {
        const struct mf_font_list_s *f = mf_get_font_list();

        debug("Font list:");
        while (f) {
            debug("full_name: %s", f->font->full_name);
            debug("short_name: %s", f->font->short_name);
            debug("size: %d %d", f->font->width, f->font->height);
            debug("x_advance: %d %d", f->font->min_x_advance, f->font->max_x_advance);
            debug("baseline: %d %d", f->font->baseline_x, f->font->baseline_y);
            debug("line_height: %d", f->font->line_height);
            debug("flags: %d", f->font->flags);
            debug("fallback_character: %c", f->font->fallback_character);
            debug("character_width: %p", f->font->character_width);
            debug("render_character: %p", f->font->render_character);

            f = f->next;
            if (f) {
                debug("");
            }
        }
    } else if (strcmp(line, "text") == 0) {
        uint16_t y_off = 0;
        const struct mf_font_list_s *f = mf_get_font_list();
        while (f) {
            struct text_font font = {
                .fontname = f->font->short_name,
                //.scale = 1,
                .font = f->font,
            };
            text_prepare_font(&font);

            struct text_conf text = {
                .text = font.fontname,
                .x = 0,
                .y = y_off,
                .justify = false,
                .alignment = MF_ALIGN_CENTER,
                .width = 240,
                .height = 240 - y_off,
                .margin = 5,
                .bg = TEXT_BG_NONE,
                .font = &font,
            };
            text_draw(&text);

            y_off = text.y;
            f = f->next;
        }
    } else if (strcmp(line, "bat") == 0) {
        draw_battery_indicator();
    } else {
        println("unknown command \"%s\"", line);
    }

    println();
}

void cnsl_init(void) {
    cnsl_buff_pos = 0;
    for (int i = 0; i < CNSL_BUFF_SIZE + 1; i++) {
        cnsl_line_buff[i] = '\0';
        cnsl_last_command[i] = '\0';
        cnsl_repeated_command[i] = '\0';
    }

#ifdef CNSL_REPEAT_PMW_STATUS_BY_DEFAULT
    strcpy(cnsl_repeated_command, "pmws");
    repeat_command = true;
#endif // CNSL_REPEAT_PMW_STATUS_BY_DEFAULT
}

static int32_t cnsl_find_line_end(void) {
    for (uint32_t i = 0; i < cnsl_buff_pos; i++) {
        if ((cnsl_line_buff[i] == '\r') || (cnsl_line_buff[i] == '\n')) {
            return i;
        }
    }
    return -1;
}

void cnsl_run(void) {
    if (repeat_command && (strlen(cnsl_repeated_command) > 0)
            && (strcmp(cnsl_repeated_command, "repeat") != 0)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now >= (last_repeat_time + CNSL_REPEAT_MS)) {
            println("repeating command \"%s\"", cnsl_repeated_command);
            cnsl_interpret(cnsl_repeated_command);
            println();

            last_repeat_time = now;
        }
    } else {
        if (repeat_command) {
            println("nothing to repeat");
        }
        repeat_command = false;
    }
}

void cnsl_handle_input(const char *buf, uint32_t len) {
    if ((cnsl_buff_pos + len) > CNSL_BUFF_SIZE) {
        debug("error: console input buffer overflow! %lu > %u", cnsl_buff_pos + len, CNSL_BUFF_SIZE);
        cnsl_init();
    }

    memcpy(cnsl_line_buff + cnsl_buff_pos, buf, len);
    cnsl_buff_pos += len;

    // handle backspace
    for (ssize_t i = cnsl_buff_pos - len; i < (ssize_t)cnsl_buff_pos; i++) {
        if ((cnsl_line_buff[i] == '\b') || (cnsl_line_buff[i] == 0x7F)) {
            if (i > 0) {
                // overwrite previous character and backspace
                for (ssize_t j = i; j < (ssize_t)cnsl_buff_pos - 1; j++) {
                    cnsl_line_buff[j - 1] = cnsl_line_buff[j + 1];
                }
                cnsl_buff_pos -= 2;
            } else {
                // just remove the backspace
                for (ssize_t j = i; j < (ssize_t)cnsl_buff_pos - 1; j++) {
                    cnsl_line_buff[j] = cnsl_line_buff[j + 1];
                }
                cnsl_buff_pos -= 1;
            }

            usb_cdc_write("\b \b", 3);

            // check for another backspace in this space
            i--;
        } else {
            usb_cdc_write(cnsl_line_buff + i, 1);
        }
    }

    int32_t line_len = cnsl_find_line_end();
    if (line_len < 0) {
        // user has not pressed enter yet
        return;
    }

    // convert line to C-style string
    cnsl_line_buff[line_len] = '\0';

    cnsl_interpret(cnsl_line_buff);

    // store command for eventual repeats
    strncpy(cnsl_last_command, cnsl_line_buff, CNSL_BUFF_SIZE + 1);

    // clear string and move following data over
    uint32_t cnt = line_len + 1;
    if (cnsl_line_buff[line_len + 1] == '\n') {
        cnt++;
    }
    memset(cnsl_line_buff, '\0', cnt);
    memmove(cnsl_line_buff, cnsl_line_buff + cnt, sizeof(cnsl_line_buff) - cnt);
    cnsl_buff_pos -= cnt;
}