#include "handler_interface.h"
#include "humidity_sensor.h"
#include "light_sensor.h"
#include "moisture_sensor.h"
#include "sensor_registers.h"
#include "temp_sensor.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct gpio_dt_spec alert_pin = GPIO_DT_SPEC_GET(DT_ALIAS(alert_gpio), gpios);

#define SLEEP_PERIOD_MS 1000
// The the time in ms one unit in SENSOR_LIGHT_TIME_THRESHOLD is, e.g. 10 * min == 600000 ms
#define TIMEOUT_UNIT_MS 600000
static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
static const struct device *sensor_i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct adc_dt_spec adc_chan = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static int32_t temperature = 42069;
static float humidity = 42069;
static uint16_t soil_moisture = 42069;
static float lux = 42069;

static volatile uint8_t low_light_timeout = 0;
static volatile int64_t deadline = 0;
static volatile uint8_t light_level_threshold = 0;
static volatile bool light_interupt_enabled = false;

int handle_read(uint8_t reg, uint8_t *to_send, size_t *bytes_to_send) {
    switch (reg) {
    case SENSOR_TEMPRATURE: {
        memcpy(to_send, &temperature, sizeof(temperature));
        *bytes_to_send = sizeof(temperature);
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
    return 0;
}

int handle_write(uint8_t reg, uint8_t val) {
    printk("[write] To 0x%02x with 0x%02x ", reg, val);
    switch (reg) {
    case SENSOR_LIGHT_LEVEL_THRESHOLD:
        light_level_threshold = val;
        printk("Light Level Threshold\n");
        break;

    case SENSOR_LIGHT_TIME_THRESHOLD:
        low_light_timeout = val;
        printk("Light Time Threshold\n");
        break;
    case SENSOR_LIGHT_INTERUPT_ENABLE:
        light_interupt_enabled = true;
        deadline = k_uptime_get() + low_light_timeout * TIMEOUT_UNIT_MS;
        printk("Light Interupt Enabled\n");
        break;
    default:
        printk("unknown\n");
        break;
    }
    return 0;
}

int main(void) {

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

    ret = gpio_pin_configure_dt(&alert_pin, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return ret;
    }

    k_msleep(3000);
    printk("init_temp_sensor: %d\n", init_temp_sensor(&adc_chan));
    printk("init_humidity_sensor: %d\n", init_humidity_sensor(sensor_i2c_bus));
    printk("init_light_sensor: %d\n", init_light_sensor(sensor_i2c_bus));
    printk("init_moisture_sensor: %d\n", init_moisture_sensor(sensor_i2c_bus));

    while (1) {
        k_msleep(SLEEP_PERIOD_MS);
        int r1 = read_temp_sensor(&temperature);
        int r2 = read_humidity_sensor(&humidity);
        int r3 = read_light_sensor(&lux);
        int r4 = read_moisture_sensor(&soil_moisture);

        if (r1 != 0) {
            printk("ERR read_temp_sensor: %d\n", r1);
        }
        if (r2 != 0) {
            printk("ERR read_humidity_sensor: %d\n", r2);
        }
        if (r3 != 0) {
            printk("ERR read_light_sensor: %d\n", r3);
        }
        if (r4 != 0) {
            printk("ERR read_moisture_sensor: %d\n", r4);
        }

        if (light_interupt_enabled) {
            int64_t now = k_uptime_get();
            printk("TIMER %lld\n", deadline - now);
            if (lux <= (float)light_level_threshold) {
                if (now >= deadline) {
                    gpio_pin_set_dt(&alert_pin, 1);
                    k_msleep(1);
                    gpio_pin_set_dt(&alert_pin, 0);
                    deadline = now + low_light_timeout * TIMEOUT_UNIT_MS;
                }
            } else {
                deadline = now + low_light_timeout * TIMEOUT_UNIT_MS;
            }
        }
    }
}