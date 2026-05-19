#include "humidity_sensor.h"
#include "light_sensor.h"
#include "moisture_sensor.h"

static const struct device *i2c_bus;

int main(void) {
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    init_humidity_sensor(i2c_bus);
    init_light_sensor(i2c_bus);
    init_moisture_sensor(i2c_bus);

    while (1) {
        uint16_t moisture;
        int l_whole, l_dec;
        int h_whole, h_dec;

        read_light_sensor(&l_whole, &l_dec);
        read_humidity_sensor(&h_whole, &h_dec);

        // int data = read_moisture_sensor(&moisture);
        read_moisture_sensor(&moisture);

        // if (data == 0) {
        printk("Moisture: %u\n", moisture);
        printk("Humidity: %d.%02d %%\n", h_whole, h_dec);
        printk("Lux: %d.%02d\n", l_whole, l_dec);
        // } else {
        // printf("Read failed: %d\n", data);
        // }

        k_msleep(3000);
    }
}