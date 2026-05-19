#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/usb/usb_device.h>

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const sensor = DEVICE_DT_GET(DT_NODELABEL(sensor_node0));

enum plant_monitor_channel {
    PLANT_MONITOR_CHAN_SOIL_MOISTURE = SENSOR_CHAN_PRIV_START,
};

static int cmd_read(const struct shell *shell, size_t argc, char **argv) {
    int ret = sensor_sample_fetch(sensor);

    if (ret < 0) {
        shell_print(shell, "Fetch failed: %d", ret);
        return ret;
    }

    struct sensor_value temp, hum, light, moisture;

    sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &hum);
    sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &light);
    sensor_channel_get(sensor, PLANT_MONITOR_CHAN_SOIL_MOISTURE, &moisture);

    if (argc < 2) {
        shell_print(shell, "Usage: read temp, moisture, hum, light or all");
        return 0;
    }

    if (!strcmp(argv[1], "temp")) {
        shell_print(shell, "Temp: %d.%06d", temp.val1, temp.val2);
        return 0;
    }

    if (!strcmp(argv[1], "hum")) {
        shell_print(shell, "Humidity: %d.%06d", hum.val1, hum.val2);
        return 0;
    }

    if (!strcmp(argv[1], "moisture")) {
        shell_print(shell, "Moisture: %d.%06d", moisture.val1, moisture.val2);
        return 0;
    }

    if (!strcmp(argv[1], "light")) {
        shell_print(shell, "Light: %d.%06d", light.val1, light.val2);
        return 0;
    }

    if (!strcmp(argv[1], "all")) {
        shell_print(shell, "Temp: %d.%06d", temp.val1, temp.val2);
        shell_print(shell, "Humidity: %d.%06d", hum.val1, hum.val2);
        shell_print(shell, "Light: %d.%06d", light.val1, light.val2);
        shell_print(shell, "Moisture: %d.%06d", moisture.val1, moisture.val2);
        return 0;
    }

    shell_print(shell, "unknown option");
    return 0;
}

SHELL_CMD_REGISTER(read, NULL, "Usage: read temp, moisture, hum, light or all", cmd_read);

int main(void) {
    uint32_t dtr = 0;

    if (usb_enable(NULL)) {
        return 0;
    }

    while (!dtr) {
        uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
        k_sleep(K_MSEC(100));
    }

    if (!device_is_ready(sensor)) {
        printk("Sensor not ready\n");
        return 0;
    }

    printk("Plant monitoring started\n");

    while (1) {
        int ret = sensor_sample_fetch(sensor);

        if (ret < 0) {
            printk("fetch error: %d\n", ret);
        } else {
            struct sensor_value light;

            sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &light);

            if (light.val1 < 10) { // TODO: Set to something reasonable
                printk("WARNING: Low light!\n");
            }
        }

        k_sleep(K_SECONDS(1));
    }
    return 0;
}