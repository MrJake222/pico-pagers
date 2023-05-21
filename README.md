# Pico Pagers

## SDK
```
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=../pico-sdk
cd ..
```

## Setup
1. Compile the server
```bash
cd pagers-server
cmake -DPICO_BOARD=pico_w .
make
```
2. Upload code to the device:
- Hold the BOOTSEL button on the Pico
- Connect the Pico to your computer
- Release the BOOTSEL button
- Copy the `pagers-server.uf2` file to the Pico

3. Connect to the Pico's serial port using USB
**macOS**
- Run `ls /dev/cu.*` to find the serial port
- Run `screen SERIAL_PORT 115200` (replace the serial port with the one you found)
- 
