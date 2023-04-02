project(pagers-common C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# this library files
add_library(pagers-common STATIC
        protocol.cpp
        physical.cpp crc.cpp)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
)

target_include_directories(pagers-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
