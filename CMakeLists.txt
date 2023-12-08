# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

# build MCUFont encoder host tool and convert included example fonts
execute_process(COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mcufont/encoder
)
execute_process(COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mcufont/fonts
)

# initialize pico-sdk from submodule
include(pico-sdk/pico_sdk_init.cmake)

project(gadget C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# copy FatFS source files to build dir, so we can use our own ffconf.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/source/ff.c
    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/ff.c
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/source/ff.h
    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/ff.h
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/source/diskio.h
    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/diskio.h
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/source/ffunicode.c
    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/ffunicode.c
    COPYONLY
)

add_executable(gadget)

target_sources(gadget PUBLIC
    src/main.c
    src/console.c
    src/log.c
    src/util.c
    src/usb.c
    src/usb_cdc.c
    src/usb_descriptors.c
    src/usb_msc.c
    src/fat_disk.c
    src/debug_disk.c
    src/buttons.c
    src/lipo.c
    src/ble.c
    src/lcd.c
    src/text.c
    src/image.c
    src/state.c
    src/volcano.c
    src/serial.c
    src/ring.c
    src/models.c
    src/state_scan.c
    src/workflow.c
    src/menu.c
    src/state_workflow.c
    src/state_volcano_run.c
    src/crafty.c
    src/state_crafty.c
    src/mem.c
    src/state_edit_workflow.c
    src/workflow_default.c
    src/state_settings.c
    src/state_about.c
    src/state_value.c
    src/textbox.c
    src/state_volcano_conf.c
    src/wifi.c
    src/venty.c
    src/state_venty.c
    src/state_wifi.c
    src/state_wifi_edit.c
    src/state_string.c

    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/ff.c
    ${CMAKE_CURRENT_BINARY_DIR}/fatfs/ffunicode.c

    st7789/src/driver_st7789.c

    mcufont/decoder/mf_encoding.c
    mcufont/decoder/mf_font.c
    mcufont/decoder/mf_justify.c
    mcufont/decoder/mf_kerning.c
    mcufont/decoder/mf_rlefont.c
    mcufont/decoder/mf_bwfont.c
    mcufont/decoder/mf_scaledfont.c
    mcufont/decoder/mf_wordwrap.c
)

# external dependency include directories
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/fatfs)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/st7789/src)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/st7789/interface)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/mcufont/decoder)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/data)
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/build)

# compress source code and stuff we want to include
add_custom_target(pack bash -c "./pack_data.sh"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS "build/pack_data.h"
)
add_dependencies(gadget pack)

# enable generous warnings
target_compile_options(gadget PUBLIC
    -Wall
    -Wextra
    -Werror
)

# suppress some warnings for borked 3rd party files in Pico SDK
set_source_files_properties(pico-sdk/lib/btstack/src/ble/sm.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(pico-sdk/lib/btstack/src/btstack_hid_parser.c PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized)
set_source_files_properties(pico-sdk/src/rp2_common/pico_cyw43_driver/cyw43_driver.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(pico-sdk/lib/btstack/src/classic/avdtp_util.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(pico-sdk/lib/btstack/src/classic/goep_client.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(pico-sdk/lib/btstack/src/classic/goep_server.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

# pull in common dependencies
target_link_libraries(gadget
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    hardware_spi
    hardware_adc
    hardware_gpio
    hardware_pwm
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_flash
    pico_flash
    picowota_reboot
)

pico_set_linker_script(gadget ${CMAKE_CURRENT_SOURCE_DIR}/src/memmap_custom.ld)

# fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
target_compile_definitions(gadget PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

set(PICOWOTA_WIFI_SSID "WIFI_SSID_HERE")
set(PICOWOTA_WIFI_PASS "WIFI_PASS_HERE")
set(PICOWOTA_WIFI_AP 0)

target_compile_definitions(gadget PUBLIC
    RUNNING_AS_CLIENT=1
    DEFAULT_WIFI_SSID="${PICOWOTA_WIFI_SSID}"
    DEFAULT_WIFI_PASS="${PICOWOTA_WIFI_PASS}"
)

add_subdirectory(picowota)
picowota_build_combined(gadget)
