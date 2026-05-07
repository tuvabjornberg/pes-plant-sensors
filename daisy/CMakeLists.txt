# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

#pico-zephyr-project
#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#-------------------------------------------------------------------------------
# NOTE: Please do not remove the #pico-zephyr-project header, it is used by
# the Raspberry Pi Pico VS Code extension to identify the project type.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(moisture-humidity LANGUAGES C)

target_include_directories(app PRIVATE src/inc)

FILE(GLOB SRC_FILES
    "src/*.c"
    "src/inc/*.c"
)

target_sources(app PRIVATE ${SRC_FILES})