project(pagers-common C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# this library files
add_library(pagers-common STATIC
        protocol.cpp
        physical.cpp crc.cpp
        lfs/lfs.c
        lfs/lfs_util.c
        fs.cpp
        circularbuffer.cpp)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_pwm
        hardware_irq
        hardware_flash
)

target_include_directories(${PROJECT_NAME} PUBLIC
        lfs
        .)

target_include_directories(pagers-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
