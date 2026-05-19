#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <string.h>

#include "handler_interface.h"

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const bus     = DEVICE_DT_GET(DT_NODELABEL(i2c1));

static int32_t temp_milli_c = 20000;

static int handle_read(uint8_t reg, uint8_t *out)
{
	uint8_t bytes[sizeof(int32_t)];

	memcpy(bytes, &temp_milli_c, sizeof(int32_t));
	if (reg < (uint8_t)sizeof(int32_t)) {
		*out = bytes[reg];
	}
	return 0;
}

static int handle_write(uint8_t reg, uint8_t val)
{
	return 0;
}

int main(void)
{
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return 0;
	}

	while (!dtr) {
		uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	}

	handler_config_t cfg = {
		.handle_read  = handle_read,
		.handle_write = handle_write,
		.address      = 0x60,
	};

	int ret = register_target_with_handlers(bus, &cfg);

	if (ret != 0) {
		printk("i2c target register failed: %d\n", ret);
		return ret;
	}

	printk("Sensor node started\n");

	int32_t counter = 0;

	while (1) {
		temp_milli_c = 20000 + (counter % 10) * 100;
		counter++;
		printk("temp: %d mC\n", (int)temp_milli_c);
		k_sleep(K_MSEC(1000));
	}

	return 0;
}