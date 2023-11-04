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
    src/debug.c
    src/buttons.c
    src/lipo.c
    src/ble.c
    src/lcd.c
    src/text.c
    src/image.c
    src/state.c

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
target_include_directories(gadget PUBLIC ${CMAKE_CURRENT_LIST_DIR}/mcufont/fonts)

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
    pico_cyw43_arch_threadsafe_background
)

target_compile_definitions(gadget PUBLIC
    RUNNING_AS_CLIENT=1
    CYW43_LWIP=0
)

# fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
target_compile_definitions(gadget PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(gadget)
