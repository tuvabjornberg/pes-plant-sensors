BOARD = rpi_pico2/rp2350a/m33
BUILD_DIR = build

.PHONY: all build flash clean

all: build

build:
	west build -b $(BOARD) .

flash:
	cp $(BUILD_DIR)/zephyr/zephyr.uf2 /Volumes/RP2350/

clean:
	rm -rf $(BUILD_DIR)

pristine:
	west build -p -b $(BOARD) .