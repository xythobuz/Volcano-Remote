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
#include <unistd.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "config.h"
#include "log.h"
#include "util.h"
#include "usb_cdc.h"
#include "usb_msc.h"
#include "debug_disk.h"
#include "lipo.h"
#include "ble.h"
#include "text.h"
#include "lcd.h"
#include "image.h"
#include "volcano.h"
#include "serial.h"
#include "main.h"
#include "models.h"
#include "workflow.h"
#include "console.h"
#include "crafty.h"

#define CNSL_BUFF_SIZE 64
#define CNSL_REPEAT_MS 500

#define DEV_AUTO_CONNECT(s) {                                     \
    ble_scan(BLE_SCAN_OFF);                                       \
    bd_addr_t addr;                                               \
    bd_addr_type_t type;                                          \
    const char *foo = s;                                          \
    sscanf(foo, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX %hhu", \
            &addr[0], &addr[1], &addr[2], &addr[3],               \
            &addr[4], &addr[5], &type);                           \
    ble_connect(addr, type);                                      \
    while (!ble_is_connected()) {                                 \
        sleep_ms(1);                                              \
    }                                                             \
}

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
        println("");
        println("  reset - reset back into this firmware");
        println("   \\x18 - reset to bootloader");
        println(" repeat - repeat last command every %d milliseconds", CNSL_REPEAT_MS);
        println("   help - print this message");
        println("  mount - make mass storage medium (un)available");
        println("  power - show Lipo battery status");
        println("");
        println("   scan - start or stop BLE scan");
        println("scanres - print list of found BLE devices");
        println("con M T - connect to (M)AC and (T)ype");
        println(" discon - disconnect from BLE device");
        println("");
        println("  clear - blank screen");
        println(" splash - draw image on screen");
        println("  fonts - show font list");
        println("   text - draw text on screen");
        println("    bat - draw battery indicator");
        println("");
        println("   vrct - Volcano read current temperature");
        println("   vrtt - Volcano read target temperature");
        println(" vwtt X - Volcano write target temperature");
        println("  vwh X - Set heater to 1 or 0");
        println("  vwp X - Set pump to 1 or 0");
        println("");
        println("    wfl - List available workflows");
        println("   wf X - Run workflow");
        println("");
        println("   crct - Crafty read current temperature");
        println("   crtt - Crafty read target temperature");
        println(" cwtt X - Crafty write target temperature");
        println("  cwh X - Set heater to 1 or 0");
        println("    crb - Crafty read battery state");
        println("");
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
        if (state && msc_is_medium_locked()) {
            println("Warning: host has locked medium. Unmounting anyway.");
        }
        msc_set_medium_available(!state);
    } else if (strcmp(line, "power") == 0) {
        float volt = lipo_voltage();
        println("Battery: %.2fV = %.1f%% @ %s",
                volt, lipo_percentage(volt),
                lipo_charging() ? "charging" : "draining");
    } else if (strcmp(line, "scan") == 0) {
        ble_scan(BLE_SCAN_TOGGLE);
    } else if (strcmp(line, "scanres") == 0) {
        struct ble_scan_result results[BLE_MAX_SCAN_RESULTS] = {0};
        int n = ble_get_scan_results(results, BLE_MAX_SCAN_RESULTS);
        if (n < 0) {
            println("Error reading results (%d)", n);
        } else {
            println("%d results", n);
            for (int i = 0; i < n; i++) {
                char info[32] = "";
                enum known_devices dev = models_filter_name(results[i].name);
                if (dev != DEV_UNKNOWN) {
                    models_get_serial(results[i].data, results[i].data_len,
                                      info, sizeof(info));
                }
                uint32_t age = to_ms_since_boot(get_absolute_time()) - results[i].time;
                println("addr=%s type=%d rssi=%d age=%.1fs name='%s' info='%s'",
                        bd_addr_to_str(results[i].addr),
                        results[i].type, results[i].rssi,
                        age / 1000.0, results[i].name, info);
            }
        }
    } else if (str_startswith(line, "con ")) {
        bd_addr_t addr;
        bd_addr_type_t type;
        int r = sscanf(line, "con %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX %hhu",
                       &addr[0], &addr[1], &addr[2], &addr[3],
                       &addr[4], &addr[5], &type);

        if (r == 7) {
            debug("connecting");
            ble_connect(addr, type);
        } else {
            debug("invalid input (%d)", r);
        }
    } else if (strcmp(line, "discon") == 0) {
        ble_disconnect();
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
    } else if (strcmp(line, "vrct") == 0) {
#ifdef TEST_VOLCANO_AUTO_CONNECT
        DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

        int16_t r = volcano_get_current_temp();
        println("volcano current temp: %.1f", r / 10.0);

#ifdef TEST_VOLCANO_AUTO_CONNECT
        ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT
    } else if (strcmp(line, "vrtt") == 0) {
#ifdef TEST_VOLCANO_AUTO_CONNECT
        DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

        int16_t r = volcano_get_target_temp();
        println("volcano target temp: %.1f", r / 10.0);

#ifdef TEST_VOLCANO_AUTO_CONNECT
        ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT
    } else if (str_startswith(line, "vwtt ")) {
        float val;
        int r = sscanf(line, "vwtt %f", &val);
        if (r != 1) {
            println("invalid input (%d)", r);
        } else {
            uint16_t v = val * 10.0;

#ifdef TEST_VOLCANO_AUTO_CONNECT
            DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

            int8_t r = volcano_set_target_temp(v);

#ifdef TEST_VOLCANO_AUTO_CONNECT
            ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT

            if (r < 0) {
                println("error writing target temp %d", r);
            } else {
                println("success");
            }
        }
    } else if (str_startswith(line, "vwh ")) {
        int val;
        int r = sscanf(line, "vwh %d", &val);
        if ((r != 1) || ((val != 0) && (val != 1))) {
            println("invalid input (%d %d)", r, val);
        } else {
#ifdef TEST_VOLCANO_AUTO_CONNECT
            DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

            int8_t r = volcano_set_heater_state(val == 1);

#ifdef TEST_VOLCANO_AUTO_CONNECT
            ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT

            if (r < 0) {
                println("error writing heater state %d", r);
            } else {
                println("success");
            }
        }
    } else if (str_startswith(line, "vwp ")) {
        int val;
        int r = sscanf(line, "vwp %d", &val);
        if ((r != 1) || ((val != 0) && (val != 1))) {
            println("invalid input (%d %d)", r, val);
        } else {
#ifdef TEST_VOLCANO_AUTO_CONNECT
            DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

            int8_t r = volcano_set_pump_state(val == 1);

#ifdef TEST_VOLCANO_AUTO_CONNECT
            ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT

            if (r < 0) {
                println("error writing pump state %d", r);
            } else {
                println("success");
            }
        }
    } else if (strcmp(line, "wfl") == 0) {
        println("%d workflows", wf_count());
        for (int i = 0; i < wf_count(); i++) {
            println("  '%s' by %s", wf_name(i), wf_author(i));
        }
    } else if (str_startswith(line, "wf ")) {
        int wf = -1;
        for (int i = 0; i < wf_count(); i++) {
            if (strcmp(wf_name(i), line + 3) == 0) {
                wf = i;
                break;
            }
        }

        if (wf < 0) {
            println("unknown workflow");
        } else {
            struct wf_state s = wf_status();
            if (s.status != WF_IDLE) {
                println("workflow in progress");
            } else {
#ifdef TEST_VOLCANO_AUTO_CONNECT
                DEV_AUTO_CONNECT(TEST_VOLCANO_AUTO_CONNECT);
#endif // TEST_VOLCANO_AUTO_CONNECT

                println("starting workflow");
                wf_start(wf);

                s = wf_status();
                while (s.status != WF_IDLE) {
                    main_loop_hw();
                    wf_run();
                    s = wf_status();
                }

                println("done");

#ifdef TEST_VOLCANO_AUTO_CONNECT
                ble_disconnect();
#endif // TEST_VOLCANO_AUTO_CONNECT
            }
        }
    } else if (strcmp(line, "crct") == 0) {
#ifdef TEST_CRAFTY_AUTO_CONNECT
        DEV_AUTO_CONNECT(TEST_CRAFTY_AUTO_CONNECT);
#endif // TEST_CRAFTY_AUTO_CONNECT

        int16_t r = crafty_get_current_temp();
        println("crafty current temp: %.1f", r / 10.0);

#ifdef TEST_CRAFTY_AUTO_CONNECT
        ble_disconnect();
#endif // TEST_CRAFTY_AUTO_CONNECT
    } else if (strcmp(line, "crtt") == 0) {
#ifdef TEST_CRAFTY_AUTO_CONNECT
        DEV_AUTO_CONNECT(TEST_CRAFTY_AUTO_CONNECT);
#endif // TEST_CRAFTY_AUTO_CONNECT

        int16_t r = crafty_get_target_temp();
        println("crafty target temp: %.1f", r / 10.0);

#ifdef TEST_CRAFTY_AUTO_CONNECT
        ble_disconnect();
#endif // TEST_CRAFTY_AUTO_CONNECT
    } else if (str_startswith(line, "cwtt ")) {
        float val;
        int r = sscanf(line, "cwtt %f", &val);
        if (r != 1) {
            println("invalid input (%d)", r);
        } else {
            uint16_t v = val * 10.0;

#ifdef TEST_CRAFTY_AUTO_CONNECT
            DEV_AUTO_CONNECT(TEST_CRAFTY_AUTO_CONNECT);
#endif // TEST_CRAFTY_AUTO_CONNECT

            int8_t r = crafty_set_target_temp(v);

#ifdef TEST_CRAFTY_AUTO_CONNECT
            ble_disconnect();
#endif // TEST_CRAFTY_AUTO_CONNECT

            if (r < 0) {
                println("error writing target temp %d", r);
            } else {
                println("success");
            }
        }
    } else if (str_startswith(line, "cwh ")) {
        int val;
        int r = sscanf(line, "cwh %d", &val);
        if ((r != 1) || ((val != 0) && (val != 1))) {
            println("invalid input (%d %d)", r, val);
        } else {
#ifdef TEST_CRAFTY_AUTO_CONNECT
            DEV_AUTO_CONNECT(TEST_CRAFTY_AUTO_CONNECT);
#endif // TEST_CRAFTY_AUTO_CONNECT

            int8_t r = crafty_set_heater_state(val == 1);

#ifdef TEST_CRAFTY_AUTO_CONNECT
            ble_disconnect();
#endif // TEST_CRAFTY_AUTO_CONNECT

            if (r < 0) {
                println("error writing heater state %d", r);
            } else {
                println("success");
            }
        }
    } else if (strcmp(line, "crb") == 0) {
#ifdef TEST_CRAFTY_AUTO_CONNECT
        DEV_AUTO_CONNECT(TEST_CRAFTY_AUTO_CONNECT);
#endif // TEST_CRAFTY_AUTO_CONNECT

        int16_t r = crafty_get_battery_state();
        println("crafty battery: %d %%", r);

#ifdef TEST_CRAFTY_AUTO_CONNECT
        ble_disconnect();
#endif // TEST_CRAFTY_AUTO_CONNECT
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

void cnsl_handle_input(const uint8_t *buf, size_t len) {
    if ((cnsl_buff_pos + len) > CNSL_BUFF_SIZE) {
        debug("error: console input buffer overflow! %lu > %u", cnsl_buff_pos + len, CNSL_BUFF_SIZE);
        cnsl_init();
    }

    memcpy(cnsl_line_buff + cnsl_buff_pos, buf, len);
    cnsl_buff_pos += len;

    // handle backspace and local echo
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

            usb_cdc_write((const uint8_t *)"\b \b", 3);
            serial_write((const uint8_t *)"\b \b", 3);

            // check for another backspace in this space
            i--;
        } else {
            usb_cdc_write((const uint8_t *)(cnsl_line_buff + i), 1);
            serial_write((const uint8_t *)(cnsl_line_buff + i), 1);
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
