#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include "moisture_sensor.h"

// Black/Red: GND, 3V3
// Green: SCL (Serial Clk)
// White: SDA (Serial Data)

int main(void)
{
    if (moisture_sensor_init() != 0) {
        printf("Sensor init failed\n");
        return -1;
    }

    while (1) {
        uint16_t moisture;

        int ret = moisture_sensor_read(&moisture);

        if (ret == 0) {
            printf("Moisture: %u\n", moisture);
        } else {
            printf("Read failed: %d\n", ret);
        }

        k_msleep(3000);
    }

    return 0;
}