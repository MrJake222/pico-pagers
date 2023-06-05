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
Run: `git clone --recurse-submodules <this repo>`

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

2. Upload code to the device:
- Hold the BOOTSEL button on the Pico
- Connect the Pico to your computer
- Release the BOOTSEL button
- Copy the `pagers-server.uf2` file to the Pico

3. Upload LittleFS image to device
- In `pagers-server` folder run: `../lfs.sh lfs [openocd|picotool]`

4. Connect to the Pico's serial port using USB
**macOS**
- Run `ls /dev/cu.*` to find the serial port
- Run `screen SERIAL_PORT 115200` (replace the serial port with the one you found)
- 

## Utilities

### LittleFS
This project uses [LittleFS](https://github.com/littlefs-project/littlefs).
To generate filesystem image and upload it to the device use `lfs.sh`. Parameters:
1. Root dir of the filesystem
2. Which tool to use to program: openocd (debug probe) or picotool (direct USB connection to target Pico)

Target address needs to be in sound with `FS_BASE_IN_FLASH` in `pagers-common/fs.cpp`.
Remember to change it when you change filesystem size.
