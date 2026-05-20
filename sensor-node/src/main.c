#include "handler_interface.h"
#include "humidity_sensor.h"
#include "light_sensor.h"
#include "moisture_sensor.h"
#include "sensor_registers.h"

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
static const struct device *sensor_i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));

static uint16_t temperature = 0;
static float humidity = 0;
static uint16_t soil_moisture = 10;
static float lux = 30;

int handle_read(uint8_t reg, uint8_t *to_send, size_t *bytes_to_send) {
    switch (reg) {
    case SENSOR_TEMPRATURE: {
        memcpy(to_send, &temperature, sizeof(temperature));
        *bytes_to_send = sizeof(uint16_t);
        break;
    }
    case SENSOR_HUMIDITY: {
        memcpy(to_send, &humidity, sizeof(humidity));
        *bytes_to_send = sizeof(humidity);
        break;
    }
    case SENSOR_SOIL: {
        memcpy(to_send, &soil_moisture, sizeof(soil_moisture));
        *bytes_to_send = sizeof(soil_moisture);
        break;
    }
    case SENSOR_LIGHT: {
        memcpy(to_send, &lux, sizeof(lux));
        *bytes_to_send = sizeof(lux);
        break;
    }

    default:
        printk("unknown\n");
        break;
    }
    *to_send = reg;
    return 0;
}

int handle_write(uint8_t reg, uint8_t val) {
    printk("[write] To 0x%02x with 0x%02x ", reg, val);
    switch (reg) {
    case SENSOR_LIGHT_LEVEL_THRESHOLD:
        printk("Light Level Threshold\n");
        break;

    case SENSOR_LIGHT_TIME_THRESHOLD:
        printk("Light Time Threshold\n");
        break;

    default:
        printk("unknown\n");
        break;
    }
    return 0;
}

int main(void) {

    init_humidity_sensor(sensor_i2c_bus);
    init_light_sensor(sensor_i2c_bus);
    init_moisture_sensor(sensor_i2c_bus);

    handler_config_t cfg = (handler_config_t){
        .handle_read = handle_read,
        .handle_write = handle_write,
        .address = 0x60,
    };

    int ret;
    if ((ret = register_target_with_handlers(bus, &cfg)) != 0) {
        printk("register: %d\n", ret);
        while (true) {
            printk("ERR: %d\n", ret);
        }
    }

    while (1) {
        k_msleep(1000);
        read_humidity_sensor(&humidity);
        read_light_sensor(&lux);
        read_moisture_sensor(&soil_moisture);
    }
}