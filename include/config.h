/*
 * config.h
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MENU_PREFER_VOLCANO
//#define MENU_PREFER_CRAFTY

#ifdef NDEBUG
// Release build

#define AUTO_MOUNT_MASS_STORAGE
#define AUTO_LOG_ON_MASS_STORAGE
#define DEBUG_DISK_WRITE_SOURCES

#ifdef MENU_PREFER_VOLCANO
#define VOLCANO_AUTO_CONNECT_TIMEOUT_MS 2000
#define VOLCANO_AUTO_CONNECT_WITHIN_MS 10000
#endif // MENU_PREFER_VOLCANO

#endif // NDEBUG

#define WATCHDOG_PERIOD_MS 1000

// ASCII 0x18 = CAN (cancel)
#define ENTER_BOOTLOADER_MAGIC 0x18

//#define DISABLE_CDC_DTR_CHECK
#define DEBOUNCE_DELAY_MS 5

#define SERIAL_WRITES_BLOCK_WHEN_BUFFER_FULL

#define DISK_BLOCK_SIZE 512

#ifdef DEBUG_DISK_WRITE_SOURCES
#define DISK_BLOCK_COUNT (256 + 128)
#else // DEBUG_DISK_WRITE_SOURCES
#define DISK_BLOCK_COUNT 256
#endif // DEBUG_DISK_WRITE_SOURCES

//#define TEST_VOLCANO_AUTO_CONNECT "xx:xx:xx:xx:xx:xx 1"
//#define TEST_CRAFTY_AUTO_CONNECT "xx:xx:xx:xx:xx:xx 0"

#endif // __CONFIG_H__
