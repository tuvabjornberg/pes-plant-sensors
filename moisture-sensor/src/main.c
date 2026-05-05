#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include <ada4026_reg.h>

#define ADA4026_ADDR 0x36

#define I2C_LABEL i2c0

// Black/Red: GND, 3V3
// Green: SCL (Serial Clk)
// White: SDA (Serial Data)

int main(void) {
    const struct device *i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

    if (i2c_bus == NULL) {
        printf("No device found; did initialization fail?\n");
        fflush(stdout);
        return -1;
    }

    if (!device_is_ready(i2c_bus)) {
        printf("I2C not ready\n");
        return -1;
    }

    printf("Everything is working.\n");
    fflush(stdout);

    uint8_t cmd[2] = {0x0F, 0x10};
    uint8_t buf[2];

    while (1) {
        i2c_write_read(i2c_bus, ADA4026_ADDR, cmd, 2, buf, 2);

        uint16_t moisture = (buf[0] << 8) | buf[1];

        printf("Moisture: %d\n", moisture);
        fflush(stdout);
        k_msleep(3000);
    }
}
