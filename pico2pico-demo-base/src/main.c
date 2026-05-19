#include <string.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define SENSOR_TEMPRATURE 0x20
#define SENSOR_HUMIDITY   0x30
#define SENSOR_SOIL       0x40
#define SENSOR_LIGHT      0x50

#define ADDR 0x60
int i2c_reg_float_read(const struct device *dev, uint16_t dev_addr, uint8_t reg_addr, float *value) {
    char read_buf[sizeof(float)];
    int res = i2c_write_read(dev, dev_addr, &reg_addr, sizeof(uint8_t), read_buf, sizeof(float));
    if (res < 0) {
        return res;
    }
    memcpy(value, read_buf, sizeof(float));
    return 0;
}
int i2c_reg_u16_read(const struct device *dev, uint16_t dev_addr, uint8_t reg_addr, uint16_t *value) {
    char read_buf[sizeof(float)];
    int res = i2c_write_read(dev, dev_addr, &reg_addr, sizeof(uint8_t), read_buf, sizeof(uint16_t));
    if (res < 0) {
        return res;
    }
    memcpy(value, read_buf, sizeof(uint16_t));
    return 0;
}

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
int main(void) {

    printk("STARTED \n");
    while (!device_is_ready(bus)) {
        printk("WAITING\n");
        k_msleep(200);
    }
    k_msleep(2000);
    printk("STARTED 2\n");
    while (true) {
        uint16_t temperature = 0;
        uint16_t soil_moisture = 0;
        float lux = 0.0;
        float humidity = 0.0;
        int temperature_res = i2c_reg_u16_read(bus, ADDR, SENSOR_TEMPRATURE, &temperature);
        printk("%d :temperature: %d units\n", temperature_res, temperature);

        int soil_moisture_res = i2c_reg_u16_read(bus, ADDR, SENSOR_SOIL, &soil_moisture);
        printk("%d :soil:        %d units\n", soil_moisture_res, soil_moisture);

        int lux_res = i2c_reg_float_read(bus, ADDR, SENSOR_LIGHT, &lux);
        printk("%d :lux:         %d.%02d\n", lux_res, (int)lux, (int)(lux * 100));

        int humidity_res = i2c_reg_float_read(bus, ADDR, SENSOR_HUMIDITY, &humidity);
        printk("%d :humidity:    %d.%02d\n", humidity_res, (int)humidity, (int)(humidity * 100));
        k_msleep(2000);
    }
}
