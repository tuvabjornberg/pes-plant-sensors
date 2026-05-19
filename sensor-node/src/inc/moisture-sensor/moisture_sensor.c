#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

#include "moisture_sensor.h"
// #include <ada4026_reg.h>

#define ADA4026_ADDR 0x36

static const struct device *i2c_bus_ref;

int init_moisture_sensor(const struct device *i2c_bus) {

    if (i2c_bus == NULL) {
        printf("No device found; did initialization fail?\n");
        fflush(stdout);
        return -1;
    }

    if (!device_is_ready(i2c_bus)) {
        printf("I2C bus not ready\n");
        return -1;
    }

    i2c_bus_ref = i2c_bus;

    printf("Everything is working.\n");
    fflush(stdout);
    return 0;
}

int read_moisture_sensor(uint16_t *moisture) {
    uint8_t cmd[2] = {0x0F, 0x10};
    uint8_t buf[2];

    int data = i2c_write_read(i2c_bus_ref, ADA4026_ADDR, cmd, sizeof(cmd), buf, sizeof(buf));

    if (data < 0) {
        return data;
    }

    *moisture = (buf[0] << 8) | buf[1];

    return 0;
}