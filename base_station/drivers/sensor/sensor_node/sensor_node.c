#define DT_DRV_COMPAT pes_sensor_node

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_node);

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

struct sensor_node_config {
    struct i2c_dt_spec i2c;
};

struct sensor_node_data {
    int32_t temp_milli_c;
    uint16_t soil_moisture;
    float humidity;
    float lux;
};

static int sensor_node_sample_fetch(const struct device *dev,
                                    enum sensor_channel chan) {
    const struct sensor_node_config *cfg = dev->config;

    uint8_t reg = 0x00;
    uint8_t bytes[sizeof(int32_t)];

    int ret = i2c_write_read_dt(&cfg->i2c, &reg, 1, bytes, sizeof(bytes));

    if (ret != 0) {
        LOG_ERR("i2c read failed: %d", ret);
        return ret;
    }

    memcpy(&data->temp_milli_c, bytes, sizeof(int32_t));
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