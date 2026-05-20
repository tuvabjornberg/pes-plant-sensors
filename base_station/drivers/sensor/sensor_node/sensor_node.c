#define DT_DRV_COMPAT pes_sensor_node

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_node);
#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#define PLANT_MONITOR_CHAN_SOIL_MOISTURE SENSOR_CHAN_PRIV_START

struct sensor_node_config {
    struct i2c_dt_spec i2c;
};

struct sensor_node_data {
    int32_t temp_milli_c;
    uint16_t soil_moisture;
    float humidity;
    float lux;
};

static int get_register(enum sensor_channel chan, struct sensor_node_data *data, uint8_t *reg, size_t *size, void **target) {
    switch (chan) {
    case SENSOR_CHAN_AMBIENT_TEMP:
        *reg = 0x20;
        *size = sizeof(int32_t);
        *target = &data->temp_milli_c;
        return 0;

    case SENSOR_CHAN_HUMIDITY:
        *reg = 0x30;
        *size = sizeof(float);
        *target = &data->humidity;
        return 0;

    case PLANT_MONITOR_CHAN_SOIL_MOISTURE:
        *reg = 0x40;
        *size = sizeof(uint16_t);
        *target = &data->soil_moisture;
        return 0;

    case SENSOR_CHAN_LIGHT:
        *reg = 0x50;
        *size = sizeof(float);
        *target = &data->lux;
        return 0;
    default:
        return -1;
    }
}
static int fetch_single_channel(const struct device *dev,
                                enum sensor_channel chan) {

    const struct sensor_node_config *cfg = dev->config;
    struct sensor_node_data *data = dev->data;

    uint8_t reg = 0x00;
    size_t reg_size = 0;
    uint8_t buffer[sizeof(float)];
    void *target = NULL;

    if (get_register(chan, data, &reg, &reg_size, &target) != 0) {
        return -ENOTSUP;
    }

    int ret = i2c_write_read_dt(&cfg->i2c, &reg, 1, buffer, reg_size);

    if (ret != 0) {
        LOG_ERR("i2c read failed: %d", ret);
        return ret;
    }

    memcpy(target, buffer, reg_size);
    return 0;
}

#define ALL_CHANNEL_COUNT 4
static int sensor_node_sample_fetch(const struct device *dev,
                                    enum sensor_channel chan) {

    const enum sensor_channel ALL_CHANNELS[ALL_CHANNEL_COUNT] = {
        SENSOR_CHAN_AMBIENT_TEMP,
        SENSOR_CHAN_HUMIDITY,
        PLANT_MONITOR_CHAN_SOIL_MOISTURE,
        SENSOR_CHAN_LIGHT,
    };

    if (chan != SENSOR_CHAN_ALL) {
        return fetch_single_channel(dev, chan);
    }

    for (size_t i = 0; i < ALL_CHANNEL_COUNT; i++) {
        int ret = fetch_single_channel(dev, ALL_CHANNELS[i]);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

static int sensor_node_channel_get(const struct device *dev,
                                   enum sensor_channel chan,
                                   struct sensor_value *val) {
    struct sensor_node_data *data = dev->data;

    if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
        return -ENOTSUP;
    }

    val->val1 = data->temp_milli_c / 1000;
    val->val2 = (data->temp_milli_c % 1000) * 1000;
    return 0;
}

static const struct sensor_driver_api sensor_node_api = {
    .sample_fetch = sensor_node_sample_fetch,
    .channel_get = sensor_node_channel_get,
};

static int sensor_node_init(const struct device *dev) {
    const struct sensor_node_config *cfg = dev->config;

    if (!i2c_is_ready_dt(&cfg->i2c)) {
        LOG_ERR("I2C not ready");
        return -ENODEV;
    }

    return 0;
}

static struct sensor_node_data sensor_node_data_inst;
static const struct sensor_node_config sensor_node_config_inst = {
    .i2c = I2C_DT_SPEC_INST_GET(0),
};

SENSOR_DEVICE_DT_INST_DEFINE(0,
                             sensor_node_init,
                             NULL,
                             &sensor_node_data_inst,
                             &sensor_node_config_inst,
                             POST_KERNEL,
                             CONFIG_SENSOR_INIT_PRIORITY,
                             &sensor_node_api);