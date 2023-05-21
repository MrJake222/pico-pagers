cmake_minimum_required(VERSION 3.13)

# pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# own libraries
add_subdirectory(../pagers-common pagers-common)

project(pagers-server C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

pico_sdk_init()

# this project private files files
add_executable(${PROJECT_NAME}
        main.cpp
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
)

pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pagers-common   # common code
        pico_cyw43_arch_lwip_threadsafe_background
)

# enable only usb stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
