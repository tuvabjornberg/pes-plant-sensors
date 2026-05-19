#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WEST_ROOT="$(cd "$SCRIPT_DIR/../../../" && pwd)"

cd "$WEST_ROOT"

echo "=== Building pico2pico-demo (sensor node) ==="
west build -p always -b rpi_pico2/rp2350a/m33 \
    --build-dir "$SCRIPT_DIR/pico2pico-demo/build" \
    "zephyr/projects/pes-plant-sensors/pico2pico-demo"

echo "=== Building pico2pico-demo-base (base station) ==="
west build -p always -b rpi_pico2/rp2350a/m33 \
    --build-dir "$SCRIPT_DIR/pico2pico-demo-base/build" \
    "zephyr/projects/pes-plant-sensors/pico2pico-demo-base"

echo "=== Done ==="
