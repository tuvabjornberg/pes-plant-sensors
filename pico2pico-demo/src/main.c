
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static char last_byte;

/*
 * @brief Callback which is called when a write request is received from the master.
 * @param config Pointer to the target configuration.
 */
int sample_target_write_requested_cb(struct i2c_target_config *config) {
    printk("sample target write requested\n");
    return 0;
}

/*
 * @brief Callback which is called when a write is received from the master.
 * @param config Pointer to the target configuration.
 * @param val The byte received from the master.
 */
int sample_target_write_received_cb(struct i2c_target_config *config, uint8_t val) {
    printk("sample target write received: 0x%02x\n", val);
    last_byte = val;
    return 0;
}

/*
 * @brief Callback which is called when a read request is received from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the byte to be sent to the master.
 */
int sample_target_read_requested_cb(struct i2c_target_config *config, uint8_t *val) {
    printk("sample target read request: 0x%02x\n", *val);
    *val = 0x42;
    return 0;
}

/*
 * @brief Callback which is called when a read is processed from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the next byte to be sent to the master.
 */
int sample_target_read_processed_cb(struct i2c_target_config *config, uint8_t *val) {
    printk("sample target read processed: 0x%02x\n", *val);
    *val = 0x43;
    return 0;
}

/*
 * @brief Callback which is called when the master sends a stop condition.
 * @param config Pointer to the target configuration.
 */
int sample_target_stop_cb(struct i2c_target_config *config) {
    printk("sample target stop callback\n");
    return 0;
}

static struct i2c_target_callbacks sample_target_callbacks = {
    .write_requested = sample_target_write_requested_cb,
    .write_received = sample_target_write_received_cb,
    .read_requested = sample_target_read_requested_cb,
    .read_processed = sample_target_read_processed_cb,
    .stop = sample_target_stop_cb,
};

int main(void) {

    struct i2c_target_config cfg = {
        .address = 0x60,
        .callbacks = &sample_target_callbacks,
    };
    printk("Target start\n");

    // while (!device_is_ready(bus)) {
    //     printk("I2C not ready\n");
    // }
    int ret;
    while ((ret = i2c_target_register(bus, &cfg)) != 0) {

        printk("register: %d\n", ret);
    }

    while (1) {
        printk("Hello\n");
        k_msleep(125);
    }
}