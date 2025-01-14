cmake_minimum_required(VERSION 3.13)

# pico w project
set(PICO_BOARD pico_w)
# for openocd debug
set(PICO_DEOPTIMIZED_DEBUG 1)

message("Using PICO_SDK_PATH: $$ENV{PICO_SDK_PATH}")

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
        http/httpserver.cpp
        http/httpserverclient.cpp
        http/mime.cpp
        pagers/pager.cpp
        pagers/pagerlist.cpp
        wififs.cpp
)

pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/http
        ${CMAKE_CURRENT_LIST_DIR}/json
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pagers-common   # common code
        pico_cyw43_arch_lwip_threadsafe_background
)

# enable usb+uart stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
