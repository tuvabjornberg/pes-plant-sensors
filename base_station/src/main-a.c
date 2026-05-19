#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const sensor  = DEVICE_DT_GET(DT_NODELABEL(sensor_node0));

int main(void)
{
	printk("!!!!\n");
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return 0;
	}

	while (!dtr) {
		uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	}

	if (!device_is_ready(sensor)) {
		printk("sensor not ready\n");
		return 0;
	}

	printk("Base station started\n");

	while (1) {
		int ret = sensor_sample_fetch(sensor);

		if (ret < 0) {
			printk("fetch error: %d\n", ret);
		} else {
			struct sensor_value temp;

			sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
			printk("temp: %d.%02d C\n", temp.val1, temp.val2 / 10000);
		}

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
