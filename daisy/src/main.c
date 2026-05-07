#include "humidity_sensor.h"
#include "moisture_sensor.h"

static const struct device *i2c_bus;

int main(void) {
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    bme_config(i2c_bus);
    moisture_sensor_init(i2c_bus);

    while (1) {
        uint16_t moisture;

        read_humidity(i2c_bus);

        int data = moisture_sensor_read(&moisture);

        if (data == 0) {
            printf("Moisture: %u\n", moisture);
        } else {
            printf("Read failed: %d\n", data);
        }

        k_msleep(3000);
    }
}