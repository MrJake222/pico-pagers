cmake_minimum_required(VERSION 3.13)


# pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# own libraries
add_subdirectory(../pagers-common pagers-common)

project(pagers-client C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# add_compile_options(-O2)

pico_sdk_init()

# this project private files files
add_executable(${PROJECT_NAME}
        main.cpp
        )

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pagers-common   # common code
        )

# enable only usb stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
