# Pi Pico Volcano Remote Control Gadget

For use with Raspberry Pi Pico W boards with the [Waveshare Pico LCD 1.3](https://www.waveshare.com/wiki/Pico-LCD-1.3) and the [Pimoroni Pico Lipo Shim](https://shop.pimoroni.com/products/pico-lipo-shim).

Adapted from the [tinyusb-cdc-example](https://github.com/hathach/tinyusb/blob/master/examples/device/cdc_msc/src/main.c), [standalone client example](https://github.com/raspberrypi/pico-examples/blob/master/pico_w/bt/standalone/client.c) and my [Trackball firmware](https://git.xythobuz.de/thomas/Trackball).

## Quick Start

When compiling for the first time, check out the required git submodules.

    git submodule update --init
    cd pico-sdk
    git submodule update --init

Then do this to build.

    mkdir build
    cd build
    cmake -DPICO_BOARD=pico_w ..
    make -j4 gadget

And flash the resulting `gadget.uf2` file to your Pico as usual.

For convenience you can use the included `flash.sh`, as long as you flashed the binary manually once before.

    make -j4 gadget
    ../flash.sh gadget.uf2

This will use the mass storage bootloader to upload a new uf2 image.

For old-school debugging a serial port will be presented by the firmware.
Open it using eg. `picocom`, or with the included `debug.sh` script.

For dependencies to compile, on Arch install these.

    sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib picocom cmake

## Proper Debugging

You can also use the SWD interface for proper hardware debugging.

This follows the instructions from the [RP2040 Getting Started document](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) from chapter 5 and 6.

For ease of reading the disassembly, create a debug build.

    mkdir build_debug
    cd build_debug
    cmake -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Debug ..
    make -j4 gadget

You need a hardware SWD probe.
This can be made from another Pico, see Appendix A in the document linked above.
For this you need to compile the `picoprobe` firmware, like this.

    git clone https://github.com/raspberrypi/picoprobe.git
    cd picoprobe
    git submodule update --init
    mkdir build
    cd build
    PICO_SDK_PATH=../../../pico-sdk cmake ..
    make -j4

And flash the resulting `picoprobe.uf2` to your probe.
Connect `GP2` of the probe to `SWCLK` of the target and `GP3` of the probe to `SWDIO` of the target.
Of course you also need to connect GND between both.

You need some dependencies, mainly `gdb-multiarch` and the RP2040 fork of `OpenOCD`.

    sudo apt install gdb-multiarch   # Debian / Ubuntu
    sudo pacman -S arm-none-eabi-gdb # Arch Linux

    cd ../.. # back to build_debug directory from before

    git clone https://github.com/raspberrypi/openocd.git --branch rp2040 --recursive --depth=1
    cd openocd

    # install udev rules
    sudo cp contrib/60-openocd.rules /etc/udev/rules.d
    sudo udevadm control --reload-rules && sudo udevadm trigger

    ./bootstrap
    ./configure --enable-ftdi --enable-sysfsgpio --enable-bcm2835gpio
    make -j4

Now we can flash a firmware image via OpenOCD.

    ./openocd/src/openocd -s openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "cmsis_dap_vid_pid 0x2e8a 0x000c" -c "program gadget.elf verify reset exit"

And also start a GDB debugging session.

    ./openocd/src/openocd -s openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "cmsis_dap_vid_pid 0x2e8a 0x000c"
    arm-none-eabi-gdb gadget.elf
    target extended-remote localhost:3333

These commands have also been put in the `flash_swd.sh` and `debug_swd.sh` scripts, respectively.
Call them from the `build_debug` folder where you checked out and built OpenOCD.

## License

The firmware itself is licensed as GPLv3.
I initially adapted it from my own [Trackball](https://git.xythobuz.de/thomas/Trackball) project.
It uses the [Pi Pico SDK](https://github.com/raspberrypi/pico-sdk), licensed as BSD 3-clause, and therefore also [TinyUSB](https://github.com/hathach/tinyusb), licensed under the MIT license.
Some code is adapted from the TinyUSB examples.
And the project uses the [FatFS library](https://github.com/abbrev/fatfs), licensed as BSD 1-clause.
Also included are the [MCUFont library](https://github.com/mcufont/mcufont) and the [st7789 library](https://github.com/hepingood/st7789), both licensed under the MIT license.
It also uses the [BTstack](https://github.com/bluekitchen/btstack/blob/master/LICENSE) included with the Pico SDK, following their [license terms](https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/pico_btstack/LICENSE.RP).

The case design is also licensed as GPLv3.
It uses a [Pi Pico case model](https://www.printables.com/model/210898-raspberry-pi-pico-case) licensed as CC-BY-NC-SA.
But this is only used for visualization purposes and doesn't influence the 3D model at all.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <http://www.gnu.org/licenses/>.
