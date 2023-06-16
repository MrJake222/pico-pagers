# Pico Pagers

## SDK
```
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=../pico-sdk
cd ..
```

## Cloning
Run:
```shell
git clone --recurse-submodules <this repo>
```

## Setup

Install `mklfs`:
```bash
git clone --recursive git@github.com:xingrz/mklfs.git
cd mklfs
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
# example adding to PATH (in mklfs dir)
export PATH=$PATH:$PWD/build
```
Remember, to add to `build` folder to the PATH!

## Server
### Running
1. Compile the server
    ```bash
    cd pagers-server
    mkdir build
    cd build
    cmake ..
    make -j4
    ```

2. Upload code to the device. You can choose one of two ways:

    - Using `picotool`:
    Run this command in the build folder. You should replace `X` with the USB device number of the Pi Pico W board.
        ```shell
        picotool load -u -v pagers-server.uf2 --address X -f
        ```

    - Using file system:
        - Hold the BOOTSEL button on the Pico
        - Connect the Pico to your computer
        - Release the BOOTSEL button
        - Copy the `pagers-server.uf2` file to the Pico

3. Upload LittleFS image to device
    - In `pagers-server` folder run: 
        ```shell
        ../lfs.sh lfs [openocd|picotool]
        ```
        If you have debug probe connected to Pico W, you should pass `openocd` as second argument. If you're directly connected via USB to Pico W, pass `picotool`.

4. Connect to the Pico's serial port using USB
    - [macOS] Run `ls /dev/cu.*` to find the serial port
    - [Linux] Run `ls /dev/ttyACM*` to find the serial port
    - Run `screen SERIAL_PORT 115200` (replace the `SERIAL_PORT` with the one you found)

## Client
### Running
1. Compile the client
    ```bash
    cd pagers-client
    mkdir build
    cd build
    cmake ..
    make -j4
    ```

2. Upload code to the device as described in the server section:
- remember to use `pagers-client.uf2` instead of `pagers-server.uf2`,
- remember to change the USB device address.

## Utility scripts
*Please note that this scripts were only tested on Linux. For other systems adjustments should be made.*

### `program.sh`
When Pico reboots (for example after flashing) the device number changes. Using `--address` option in `picotool` quickly becomes annoying. Also keeping track of device numbers and project names also is difficult and error-prone. So `program.sh` script was written. It takes whats constans between reboots: **USB port number** (denoted as `busport` in script). Then it takes device number from `/sys/bus/usb/devices/$busport/devnum` (it might be different on other systems) and uses this as the `--address` parameter to `picotool`. Argument list:
1. USB port number
2. Project name: `pagers-server` or `pagers-client`
3. Project type: `release` or `debug`

Then it programs the device on specified port with file from `<project name>/cmake-build-<project type>/<project name>.uf2`. This is CLion output directory (again, might want to change this one).
### `lfs.sh`
This project uses [LittleFS](https://github.com/littlefs-project/littlefs).
To generate filesystem image and upload it to the device use `lfs.sh`. Parameters:
1. Root dir of the filesystem
2. Which tool to use to program: openocd (debug probe) or picotool (direct USB connection to target Pico)
2. [Linux] Which USB port number to use (same working principle as in `program.sh`)

Target address needs to be in sound with `FS_BASE_IN_FLASH` in `pagers-common/fs.cpp`.
Remember to change it when you change filesystem size.
