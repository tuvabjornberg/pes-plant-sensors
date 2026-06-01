#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/usb/usb_device.h>

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const sensor = DEVICE_DT_GET(DT_NODELABEL(sensor_node0));

#define SENSOR_ATTR_THRESHOLD_DUR SENSOR_ATTR_PRIV_START

enum plant_monitor_channel {
    PLANT_MONITOR_CHAN_SOIL_MOISTURE = SENSOR_CHAN_PRIV_START,
};
static uint8_t current_period_min = 0;

static void periodic_read_handler(struct k_work *work) {
    struct sensor_value val;

    printk("\n--- Periodic Sensor Read ---\n");

    if (sensor_sample_fetch_chan(sensor, SENSOR_CHAN_AMBIENT_TEMP) == 0) {
        sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &val);
        printk("Temp: %d.%06d\n", val.val1, val.val2);
    }

    if (sensor_sample_fetch_chan(sensor, SENSOR_CHAN_HUMIDITY) == 0) {
        sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &val);
        printk("Humidity: %d.%06d\n", val.val1, val.val2);
    }

    if (sensor_sample_fetch_chan(sensor, PLANT_MONITOR_CHAN_SOIL_MOISTURE) == 0) {
        sensor_channel_get(sensor, PLANT_MONITOR_CHAN_SOIL_MOISTURE, &val);
        printk("Moisture: %d.%06d\n", val.val1, val.val2);
    }

    if (sensor_sample_fetch_chan(sensor, SENSOR_CHAN_LIGHT) == 0) {
        sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &val);
        printk("Light: %d.%06d\n", val.val1, val.val2);
    }
}

K_WORK_DEFINE(periodic_read_work, periodic_read_handler);

static void timer_handler(struct k_timer *dummy) {
    k_work_submit(&periodic_read_work);
}

// Define the hardware-backed timer
K_TIMER_DEFINE(periodic_timer, timer_handler, NULL);

// 3. Command Handlers
static int cmd_start(const struct shell *shell, size_t argc, char **argv) {
    if (argc < 2) {
        shell_print(shell, "Usage: start <period_min>");
        return -EINVAL;
    }

    int period = atoi(argv[1]);
    if (period <= 0 || period > 255) {
        shell_print(shell, "Invalid period. Must be between 1 and 255.");
        return -EINVAL;
    }

    current_period_min = (uint8_t)period;

    // Start the timer: First tick happens K_NO_WAIT (immediately),
    // and it auto-reloads exactly every K_MINUTES(current_period_min)
    k_timer_start(&periodic_timer, K_NO_WAIT, K_MINUTES(current_period_min));

    shell_print(shell, "Started reading every %d minute(s).", current_period_min);
    return 0;
}

static int cmd_stop(const struct shell *shell, size_t argc, char **argv) {
    // Stop the hardware timer
    k_timer_stop(&periodic_timer);

    // Cancel any work that the timer might have just submitted but hasn't run yet
    k_work_cancel(&periodic_read_work);

    shell_print(shell, "Stopped periodic reading.");
    return 0;
}

static int cmd_read(const struct shell *shell, size_t argc, char **argv) {

    if (argc < 2) {
        shell_print(shell, "Usage: read temp, moisture, hum, light or all");
        return 0;
    }

    bool is_all = !strcmp(argv[1], "all");
    bool any_matched = is_all;
    if (is_all || !strcmp(argv[1], "temp")) {
        struct sensor_value temp;
        int ret = sensor_sample_fetch_chan(sensor, SENSOR_CHAN_AMBIENT_TEMP);
        if (ret < 0) {
            shell_print(shell, "Fetch failed: %d", ret);
            return ret;
        }
        sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        shell_print(shell, "Temp: %d.%06d", temp.val1, temp.val2);
        any_matched = true;
    }

    if (is_all || !strcmp(argv[1], "hum")) {
        struct sensor_value hum;
        int ret = sensor_sample_fetch_chan(sensor, SENSOR_CHAN_HUMIDITY);
        if (ret < 0) {
            shell_print(shell, "Fetch failed: %d", ret);
            return ret;
        }
        sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &hum);

        shell_print(shell, "Humidity: %d.%06d", hum.val1, hum.val2);
        any_matched = true;
    }

    if (is_all || !strcmp(argv[1], "moisture")) {
        struct sensor_value moisture;
        int ret = sensor_sample_fetch_chan(sensor, PLANT_MONITOR_CHAN_SOIL_MOISTURE);
        if (ret < 0) {
            shell_print(shell, "Fetch failed: %d", ret);
            return ret;
        }
        sensor_channel_get(sensor, PLANT_MONITOR_CHAN_SOIL_MOISTURE, &moisture);
        shell_print(shell, "Moisture: %d.%06d", moisture.val1, moisture.val2);
        any_matched = true;
    }

    if (is_all || !strcmp(argv[1], "light")) {
        struct sensor_value light;
        int ret = sensor_sample_fetch_chan(sensor, SENSOR_CHAN_LIGHT);
        if (ret < 0) {
            shell_print(shell, "Fetch failed: %d", ret);
        }

        sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &light);
        shell_print(shell, "Light: %d.%06d", light.val1, light.val2);
        any_matched = true;
    }

    if (!any_matched) {
        shell_print(shell, "unknown option");
    }
    return 0;
}

static void cmd_set(const struct shell *shell, size_t argc, char **argv) {
    if (argc < 3) {
        shell_print(shell, "Usage: set {threshold, or min} {0 < n < 256}");
        return;
    }

    int parsed = atoi(argv[2]);
    enum sensor_attribute attr;
    if (!strcmp(argv[1], "threshold")) {
        attr = SENSOR_ATTR_UPPER_THRESH;
    } else if (!strcmp(argv[1], "min")) {
        attr = SENSOR_ATTR_THRESHOLD_DUR;
    } else {
        shell_print(shell, "unknown option");
        return;
    }

    if (parsed <= 0 || 256 <= parsed) {
        shell_print(shell, "Invalid n");
        shell_print(shell, "Usage: set {threshold, or min} {0 < n < 256}");
        return;
    }
    struct sensor_value val = {.val1 = parsed, .val2 = 0};
    sensor_attr_set(sensor, SENSOR_CHAN_LIGHT, attr, &val);
}

K_SEM_DEFINE(interupt_waker, 0, 1);

static void send_message(const struct device *dev, const struct sensor_trigger *trigger) {
    k_sem_give(&interupt_waker);
}

static void cmd_enable(const struct shell *shell, size_t argc, char **argv) {
    static const struct sensor_trigger trig = {
        .type = SENSOR_TRIG_THRESHOLD,
        .chan = SENSOR_CHAN_LIGHT, // Specific channel
    };
    sensor_trigger_set(sensor, &trig, send_message);
}

SHELL_CMD_REGISTER(read, NULL, "Usage: read temp, moisture, hum, light or all", cmd_read);
SHELL_CMD_REGISTER(set, NULL, "Usage: set <threshold, or min> <0 <= n <256>", cmd_set);
SHELL_CMD_REGISTER(enable, NULL, "Usage: enable", cmd_enable);
SHELL_CMD_REGISTER(start, NULL, "Usage: start <period_min>", cmd_start);
SHELL_CMD_REGISTER(stop, NULL, "Usage: stop", cmd_stop);

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
        k_sem_take(&interupt_waker, K_FOREVER);
        printk("WARNING: Low light!\n");
    }
    return 0;
}