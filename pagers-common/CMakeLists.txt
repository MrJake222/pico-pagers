project(pagers-common C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# this library files
add_library(pagers-common STATIC
        protocol.cpp
)

target_include_directories(pagers-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
