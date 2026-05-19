#include "inc/humidity-sensor/humidity-sensor.h"
#include "inc/light-sensor/light-sensor.h"

extern struct k_sem low_light_sem; 

static const struct device *i2c_bus;

#define LIGHT_ALERT_STACK_SIZE 1024

K_THREAD_STACK_DEFINE(light_alert_stack, LIGHT_ALERT_STACK_SIZE);
static struct k_thread light_alert_thread_data;

static void light_alert_thread(void *a, void *b, void *c) {
    while (1) {
        k_sem_take(&low_light_sem, K_FOREVER);
        printk("Alert: light level too low!\n");
    }
}

int main(void) {
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));

    init_light_sensor(i2c_bus);
    init_humidity_sensor(i2c_bus);

    k_thread_create(&light_alert_thread_data, light_alert_stack,
                    K_THREAD_STACK_SIZEOF(light_alert_stack),
                    light_alert_thread,
                    NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    while (1) {
        read_light_sensor(i2c_bus);
        read_humidity_sensor(i2c_bus);
        k_msleep(1000);
    }
}