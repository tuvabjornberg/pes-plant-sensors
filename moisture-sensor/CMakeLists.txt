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

cmake_minimum_required(VERSION 3.20.0)

# Select our board 
set(BOARD rpi_pico2/rp2350a/m33)

# Find/Select cmake package 'Zephyr' 
find_package(Zephyr)

# This is only used by IntelliSense inside VS Code 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# For using west flash with picoprobe
set(OPENOCD "openocd")
set(OPENOCD_DEFAULT_PATH "/usr/local/share/openocd/scripts")

# Define project name
project(hum-sensor)

# Add include directory 
target_include_directories(app PRIVATE src/inc)

# Add source files
FILE(GLOB SRC_FILES "src/*.c")
target_sources(app PRIVATE ${SRC_FILES})

