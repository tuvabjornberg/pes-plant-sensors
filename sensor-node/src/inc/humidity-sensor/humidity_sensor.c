#include "humidity_sensor.h"

#define BME680_ADDR BME68X_I2C_ADDR_HIGH

static struct bme68x_dev bme;
static uint8_t dev_addr = BME68X_I2C_ADDR_HIGH;
const struct device *i2c_bus_shared;

static BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                                            uint32_t len, void *intf_ptr) {
    uint8_t addr = *(uint8_t *)intf_ptr;
    return i2c_write_read(i2c_bus_shared, addr, &reg_addr, 1, reg_data, len);
}

static BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr,
                                             const uint8_t *reg_data,
                                             uint32_t len, void *intf_ptr) {
    uint8_t addr = *(uint8_t *)intf_ptr;

    uint8_t buf[len + 1];
    buf[0] = reg_addr;
    memcpy(&buf[1], reg_data, len);
    return i2c_write(i2c_bus_shared, buf, len + 1, addr);
}

static void bme68x_delay_us(uint32_t period, void *intf_ptr) {
    k_sleep(K_USEC(period));
}

int init_humidity_sensor(const struct device *i2c_bus) {
    int res;
    i2c_bus_shared = i2c_bus;

    bme.read = bme68x_i2c_read;
    bme.write = bme68x_i2c_write;
    bme.delay_us = bme68x_delay_us;
    bme.intf = BME68X_I2C_INTF;
    bme.intf_ptr = &dev_addr;

    res = bme68x_init(&bme);
    if (res < 0) {
        return res;
    }

    struct bme68x_conf conf;
    res = bme68x_get_conf(&conf, &bme);
    if (res < 0) {
        return res;
    }
    conf.os_hum = BME68X_OS_2X;
    conf.os_temp = BME68X_OS_8X;
    conf.os_pres = BME68X_OS_NONE;
    conf.filter = BME68X_FILTER_OFF;
    res = bme68x_set_conf(&conf, &bme);
    if (res < 0) {
        return res;
    }

    struct bme68x_heatr_conf heatr_conf = {0};
    heatr_conf.enable = BME68X_DISABLE;
    return bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
}

int read_humidity_sensor(float *humidity) {
    struct bme68x_conf conf;

    int res = bme68x_get_conf(&conf, &bme);
    if (res < 0) {
        return res;
    }

    res = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    if (res < 0) {
        return res;
    }

    uint32_t delay_us = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme);
    k_sleep(K_USEC(delay_us));

    struct bme68x_data data;
    uint8_t n_fields;
    res = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
    if (res < 0) {
        return res;
    }
    *humidity = data.humidity;
    return 0;
}