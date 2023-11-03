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
        println("Trackball Firmware Usage:");
        println("  reset - reset back into this firmware");
        println("   \\x18 - reset to bootloader");
        println(" repeat - repeat last command every %d milliseconds", CNSL_REPEAT_MS);
        println("   help - print this message");
        println("  mount - make mass storage medium (un)available");
        //println("    foo - bar");
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
