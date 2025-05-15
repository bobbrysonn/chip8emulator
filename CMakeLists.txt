cmake_minimum_required(VERSION 3.20)

project(Chip8Emulator
        VERSION 1.0
        DESCRIPTION "A Chip8 Emulator"
        LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
        src/main.c
        src/chip8.h
        src/chip8.c
)

find_package(SDL3 CONFIG REQUIRED)

add_executable(Chip8Emulator ${SOURCES})

target_link_libraries(Chip8Emulator PRIVATE SDL3::SDL3)