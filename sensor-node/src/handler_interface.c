#include "handler_interface.h"
#include <stdalign.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
// Implementation of
// https://mermaid.live/edit#pako:eNp1kktPwzAMgP9K5CPqRpd2feSAxOOKhMoBCYqmsLhdxZqUPMZg2n8nXbfBBORkO99nS4k3MFcCgUEpjeUWbxpea96OVrSUxJ-ns2cyGl3cW9V1KIbaPunr5EE3Fgt8c2gsCnaSD_Qp0TuXQugC5yuyxwfuWO2Rw4g9Qc7JgkuxxJnGevb-j1Sg6ZQUV67yXoFceM2gdR15cVWFmlhF7HoQf7Cn8_roT-RX9zut5miMt0pZSgig1o0AZrXDAFrULe9T2PTdSrALbLEE5kPB9WvpH3zrnY7LR6Xag6aVqxfAKr40PnOd-P6SI4JSoL5WTlpgaRbvegDbwBoYTabjhOZRMonzKMr7yw9gcTTO85AmeTwJwyxJtwF87maG4zSdZNM8itOMhjSjAaBorNK3w07sVmP7BcyqreI
enum COMM_STATE { COMM_STOPPED,
                  COMM_WRITE_REQUESTED,
                  COMM_ADDR_RECIVED,
                  COMM_BUF_SEND };

typedef struct comm_state_machine {
    alignas(float) char send_buf[MAX_BUF_SIZE];
    size_t msg_len; // must be <= MAX_BUF_SIZE
    int cursor;     // must be < MAX_BUF_SIZE
    uint8_t addr;
    enum COMM_STATE state;

} comm_state_machine_t;

static comm_state_machine_t comm = (comm_state_machine_t){
    .state = COMM_STOPPED,
    .addr = 0,
};

static handler_config_t *registered_config = NULL;

static int write_requested_cb(struct i2c_target_config *config) {
    comm.state = COMM_WRITE_REQUESTED;
    return 0;
}

static int write_received_cb(struct i2c_target_config *config, uint8_t val) {

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

    if (comm.state == COMM_ADDR_RECIVED) {
        comm.msg_len = 0;
        registered_config->handle_read(comm.addr, comm.send_buf, &comm.msg_len);
        *out = comm.send_buf[0];
        comm.state = COMM_BUF_SEND;
        printk("read request: %d  : %02x %02x %02x %02x\n", comm.msg_len, comm.send_buf[0], comm.send_buf[1], comm.send_buf[2], comm.send_buf[3]);
        comm.cursor = 1;
    } else {
        printk("Unexpected read: 0x%02x\n", *out);
    }

    return 0;
}

static int read_processed_cb(struct i2c_target_config *config, uint8_t *val) {
    // TOOO:figure out what all this means
    if (comm.state == COMM_BUF_SEND && comm.cursor < comm.msg_len) {
        *val = comm.send_buf[comm.cursor];
        comm.cursor++;
    } else {
        *val = 0xff;
    }

    printk("read processed: 0x%02x\n", *val);
    return 0;
}

static int stop_cb(struct i2c_target_config *config) {
    printk("sample target stop callback\n");
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
