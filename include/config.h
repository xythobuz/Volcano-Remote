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

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 3

#define MENU_PREFER_VOLCANO
//#define MENU_PREFER_CRAFTY

#define VOLCANO_AUTO_CONNECT_TIMEOUT_MS 2000
#define VOLCANO_AUTO_CONNECT_WITHIN_MS 10000

#define COUNTRY_CODE CYW43_COUNTRY_GERMANY

#ifdef NDEBUG
// Release build
#define AUTO_MOUNT_MASS_STORAGE
#define AUTO_LOG_ON_MASS_STORAGE
#endif // NDEBUG

//#define INFLUXDB_HOST "IP_V4_HERE"
//#define INFLUXDB_PORT 8086
//#define INFLUXDB_DATABASE "db_name"

#define WATCHDOG_PERIOD_MS 1000
#define FLASH_LOCK_TIMEOUT_MS 500

// ASCII 0x18 = CAN (cancel)
#define ENTER_BOOTLOADER_MAGIC 0x18

//#define DISABLE_CDC_DTR_CHECK
#define DEBOUNCE_DELAY_MS 5

#define SERIAL_WRITES_BLOCK_WHEN_BUFFER_FULL

// TODO needs to be the same as in pack_data.sh
#define DISK_BLOCK_SIZE 512
#define DISK_BLOCK_COUNT (256 + 128) // 384 * 512 = 196608

//#define TEST_VOLCANO_AUTO_CONNECT "xx:xx:xx:xx:xx:xx 1"
//#define TEST_CRAFTY_AUTO_CONNECT "xx:xx:xx:xx:xx:xx 0"

#endif // __CONFIG_H__
