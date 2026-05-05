cmake_minimum_required(VERSION 3.20.0)

# Register this directory so Zephyr finds dts/bindings/sensor/pes,sensor-node.yaml
list(APPEND DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(base-station)

target_sources(app PRIVATE
  src/main.c
  drivers/sensor/sensor_node/sensor_node.c)

target_include_directories(app PRIVATE drivers/sensor/sensor_node)
