#include "handler_interface.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
int handle_read(uint8_t reg, uint8_t *out) {
    printk("[read] From 0x%02x\n", reg);
    *out = reg;
    return 0;
}

int handle_write(uint8_t reg, uint8_t val) {
    printk("[write] To 0x%02x with 0x%02x\n", reg, val);
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

    while (1) {
        k_msleep(1000);
    }
}