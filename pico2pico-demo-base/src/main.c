
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define ADDR 0x60

static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
#define BUF_LEN 6
int main(void) {

    char buf[BUF_LEN] = "HELLO";
    while (!device_is_ready(bus)) {
        printk("WAITING\n");
        k_msleep(200);
    }
    k_msleep(5000);

    for (size_t i = 0; i < 256; i++) {
        int sc = i2c_write(bus, buf, BUF_LEN, ADDR);
        int sc2 = i2c_read(bus, buf, BUF_LEN, ADDR);
        printk("AjSD %d %d %d\n", i, sc, sc2);
        k_msleep(500);
    }

    return 0;
}