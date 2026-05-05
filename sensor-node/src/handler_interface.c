#include "handler_interface.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

enum COMM_STATE { COMM_STOPPED,
                  COMM_WRITE_REQUESTED,
                  COMM_ADDR_RECIVED };

typedef struct comm_state_machine {
    enum COMM_STATE state;
    uint8_t addr;
} comm_state_machine_t;

static comm_state_machine_t comm = (comm_state_machine_t){
    .state = COMM_STOPPED,
    .addr = 0,
};

static handler_config_t *registered_config = NULL;

static int write_requested_cb(struct i2c_target_config *config) {
    printk("write requested\n");
    comm.state = COMM_WRITE_REQUESTED;
    return 0;
}

static int write_received_cb(struct i2c_target_config *config, uint8_t val) {
    // printk("write received: 0x%02x\n", val);

    switch (comm.state) {
    case COMM_WRITE_REQUESTED:
        comm.addr = val;
        comm.state = COMM_ADDR_RECIVED;
        break;

    case COMM_ADDR_RECIVED:
        registered_config->handle_write(comm.addr, val);
        break;
    default:
        printk("Unexpected write received\n");
        break;
    }

    return 0;
}

static int read_requested_cb(struct i2c_target_config *config, uint8_t *out) {
    // printk("read request: 0x%02x\n", *out);

    if (comm.state == COMM_ADDR_RECIVED) {
        registered_config->handle_read(comm.addr, out);
    } else {
        printk("Unexpected read: 0x%02x\n", *out);
    }

    return 0;
}

static int read_processed_cb(struct i2c_target_config *config, uint8_t *val) {
    // TOOO:figure out what all this means
    // printk("read processed: 0x%02x\n", *val);
    *val = 0x43;
    return 0;
}

static int stop_cb(struct i2c_target_config *config) {
    // printk("sample target stop callback\n");
    comm.state = COMM_STOPPED;
    return 0;
}

static void error_cb(struct i2c_target_config *config, enum i2c_error_reason error_code) {
    printk("target error %d\n", error_code);
}

static struct i2c_target_callbacks callbacks = {
    .write_requested = write_requested_cb,
    .write_received = write_received_cb,
    .read_requested = read_requested_cb,
    .read_processed = read_processed_cb,
    .stop = stop_cb,
    .error = error_cb};

static struct i2c_target_config i2c_cfg = {
    .address = 0x60,
    .callbacks = &callbacks,
    .flags = 0,
};

int register_target_with_handlers(const struct device *dev, handler_config_t *cfg) {
    if (cfg == NULL || registered_config != NULL) {
        return -EINVAL;
    }

    i2c_cfg.address = cfg->address;

    int ret = i2c_target_register(dev, &i2c_cfg);
    if (ret != 0) {
        return ret;
    }

    registered_config = cfg;
    return 0;
}
