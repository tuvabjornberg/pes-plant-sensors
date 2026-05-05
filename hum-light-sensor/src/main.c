#include "inc/humidity-sensor/humidity-sensor.h"
#include "inc/light-sensor/light-sensor.h"

static const struct device *i2c_bus;

int main(void) {
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));



    init_light_sensor(i2c_bus);
    bme_config(i2c_bus);

    while (1) {
        read_light_sensor(i2c_bus);
        read_humidity(i2c_bus);
        k_msleep(500);
    }
}