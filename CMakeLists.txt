cmake_minimum_required(VERSION 3.21)

project(minwm_proto
    LANGUAGES CXX
)

if (WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project must be built as 64-bit (x64).")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(winwm
    ${SOURCES}
)

target_include_directories(winwm PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

# windows deps
target_link_libraries(winwm PRIVATE
    gdi32
    user32
    dwmapi
)

if (MSVC)
    target_compile_options(winwm PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(winwm PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
