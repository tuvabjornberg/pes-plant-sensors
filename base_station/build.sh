#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WEST_ROOT="$(cd "$SCRIPT_DIR/../../../../" && pwd)"
SOURCE_DIR="zephyr/projects/pes-plant-sensors/base_station"

cd "$WEST_ROOT"
west build -p always -b rpi_pico2/rp2350a/m33 --build-dir "$SCRIPT_DIR/build" "$SOURCE_DIR"
