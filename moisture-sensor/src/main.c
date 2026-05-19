#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include "moisture_sensor.h"

#define I2C_LABEL    i2c0

// Black/Red: GND, 3V3
// Green: SCL (Serial Clk)
// White: SDA (Serial Data)

static const struct device *i2c_bus;

int main(void) {
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

    if (moisture_sensor_init(i2c_bus) != 0) {
        printf("Sensor init failed\n");
        return -1;
    }

    while (1) {
        uint16_t moisture;

        int data = moisture_sensor_read(&moisture);

        if (data == 0) {
            printf("Moisture: %u\n", moisture);
        } else {
            printf("Read failed: %d\n", data);
        }

        k_msleep(3000);
    }

    return 0;
}