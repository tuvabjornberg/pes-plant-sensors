cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(hum-light-sensor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_sources(app PRIVATE 
    src/main.c 
    src/inc/light-sensor/light-sensor.c 
    src/inc/humidity-sensor/humidity-sensor.c
)

FILE(GLOB SRC_FILES "src/*.c")
target_sources(app PRIVATE ${SRC_FILES})