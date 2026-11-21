cmake_minimum_required(VERSION 3.20)
project(nicmon VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(nicmon src/nicmon.c)

target_link_libraries(nicmon iphlpapi ws2_32)

if(MSVC)
    target_compile_options(nicmon PRIVATE /W4 /O2)
endif()