#define DT_DRV_COMPAT pes_sensor_node

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_node);
#include "sensor_node.h"
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#define PLANT_MONITOR_CHAN_SOIL_MOISTURE SENSOR_CHAN_PRIV_START

#define LIGHT_LEVEL_THESHOLD_REG     0x51
#define LIGHT_LEVEL_THESHOLD_DUR_REG 0x52
#define LIGHT_INTERUPT_ENABLE_REG    0x53

struct sensor_node_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec int_gpio;
};

struct sensor_node_data {
    sensor_trigger_handler_t handler;
    const struct sensor_trigger *trigger;
    const struct device *dev;
    struct gpio_callback gpio_cb;
    struct k_work work;
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

    LOG_ERR("0x%x, size:%d", reg, reg_size);
    int ret = i2c_write_read_dt(&cfg->i2c, &reg, 1, buffer, reg_size);
    if (ret != 0) {
        LOG_ERR("i2c read failed: %d", ret);
        return ret;
    }

    memcpy(target, buffer, reg_size);
    if (reg == 0x40) {
        LOG_ERR("%d 0x%02x 0x%02x\n", data->soil_moisture, buffer[0], buffer[1]);
    }
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

    switch (chan) {
    case SENSOR_CHAN_AMBIENT_TEMP:
        return sensor_value_from_milli(val, data->temp_milli_c);
    case SENSOR_CHAN_HUMIDITY:
        return sensor_value_from_float(val, data->humidity);

    case PLANT_MONITOR_CHAN_SOIL_MOISTURE:
        val->val1 = data->soil_moisture;
        val->val2 = 0;
        return 0;

    case SENSOR_CHAN_LIGHT:
        return sensor_value_from_float(val, data->lux);
    default:
        return -ENOTSUP;
    }

    return 0;
}

static int sensor_node_trigger_set(const struct device *dev,
                                   const struct sensor_trigger *trig,
                                   sensor_trigger_handler_t handler) {
    struct sensor_node_data *data = dev->data;
    const struct sensor_node_config *config = dev->config;

    if (trig->type != SENSOR_TRIG_THRESHOLD) {
        return -ENOTSUP;
    }

    gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_DISABLE);
    data->trigger = trig;
    data->handler = handler;

    i2c_reg_write_byte_dt(&config->i2c, LIGHT_INTERUPT_ENABLE_REG, 1);

    /* Re-enable the GPIO interrupt if a handler was provided */
    if (handler != NULL) {
        gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);
    }

    return 0;
}
static int sensor_node_attr_set(const struct device *dev,
                                enum sensor_channel chan,
                                enum sensor_attribute attr,
                                const struct sensor_value *val) {
    const struct sensor_node_config *config = dev->config;
    if (val->val1 < 0 || val->val1 >= 256 || val->val2 != 0) {
        return -EINVAL;
    }

    switch (chan) {
    case SENSOR_CHAN_LIGHT:
        switch (attr) {
        case SENSOR_ATTR_UPPER_THRESH:
            return i2c_reg_write_byte_dt(&config->i2c, LIGHT_LEVEL_THESHOLD_REG, (uint8_t)val->val1);
        case SENSOR_ATTR_THRESHOLD_DUR:
            return i2c_reg_write_byte_dt(&config->i2c, LIGHT_LEVEL_THESHOLD_DUR_REG, (uint8_t)val->val1);
        default:
            return -ENOTSUP;
        }

    default:
        return -ENOTSUP;
    }
}
static void sensor_node_work_cb(struct k_work *work) {
    struct sensor_node_data *data = CONTAINER_OF(work, struct sensor_node_data, work);

    if (data->handler != NULL) {
        data->handler(data->dev, data->trigger);
    }
}

static void sensor_node_gpio_cb(const struct device *port,
                                struct gpio_callback *cb,
                                uint32_t pins) {
    struct sensor_node_data *data = CONTAINER_OF(cb, struct sensor_node_data, gpio_cb);

    k_work_submit(&data->work);
}

static const struct sensor_driver_api sensor_node_api = {
    .sample_fetch = sensor_node_sample_fetch,
    .channel_get = sensor_node_channel_get,
    .trigger_set = sensor_node_trigger_set,
    .attr_set = sensor_node_attr_set,
};

static int sensor_node_init(const struct device *dev) {
    const struct sensor_node_config *cfg = dev->config;
    struct sensor_node_data *data = dev->data;
    data->dev = dev;

    if (!i2c_is_ready_dt(&cfg->i2c)) {
        LOG_ERR("I2C not ready");
        return -ENODEV;
    }

    k_work_init(&data->work, sensor_node_work_cb);

    /* Check if the GPIO is ready and configure it as an input */
    if (!gpio_is_ready_dt(&cfg->int_gpio)) {
        return -ENODEV;
    }

    gpio_pin_configure_dt(&cfg->int_gpio, GPIO_INPUT);

    /* Set up the GPIO callback */
    gpio_init_callback(&data->gpio_cb, sensor_node_gpio_cb, BIT(cfg->int_gpio.pin));
    gpio_add_callback(cfg->int_gpio.port, &data->gpio_cb);
    return 0;
}

static struct sensor_node_data sensor_node_data_inst;
static const struct sensor_node_config sensor_node_config_inst = {
    .i2c = I2C_DT_SPEC_INST_GET(0),
    .int_gpio = GPIO_DT_SPEC_INST_GET_OR(0, irq_gpios, {0}),
};

SENSOR_DEVICE_DT_INST_DEFINE(0,
                             sensor_node_init,
                             NULL,
                             &sensor_node_data_inst,
                             &sensor_node_config_inst,
                             POST_KERNEL,
                             CONFIG_SENSOR_INIT_PRIORITY,
                             &sensor_node_api);