cmake_minimum_required(VERSION 3.20.0)



find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(light-sensor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_include_directories(app PRIVATE src/inc)


FILE(GLOB SRC_FILES "src/*.c")
target_sources(app PRIVATE ${SRC_FILES})