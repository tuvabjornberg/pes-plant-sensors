
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define ADDR 0x60

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c1));
int main(void) {

    printk("STARTED \n");
    while (!device_is_ready(bus)) {
        printk("WAITING\n");
        k_msleep(200);
    }
    k_msleep(5000);
    printk("STARTED 2j\n");

    for (size_t i = 0; i < 10; i++) {
        uint8_t out = 0;
        int sc = i2c_reg_read_byte(bus, ADDR, (uint8_t)i, &out);
        printk("Read %d %d out:%d\n", i, sc, out);
        k_msleep(500);
    }

    for (size_t i = 0; i < 20; i++) {
        int sc = i2c_reg_write_byte(bus, ADDR, (uint8_t)i, 100 + i);
        printk("Wrote %d %d \n", i, sc);
        k_msleep(500);
    }

    return 0;
}