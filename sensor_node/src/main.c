#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <string.h>
#include <errno.h>

#include "handler_interface.h"

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const bus     = DEVICE_DT_GET(DT_NODELABEL(i2c1));

static const struct adc_dt_spec adc_chan =
	ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static struct sensor_value temp_value;

static int get_temp(struct sensor_value *out)
{
	int16_t sample;
	struct adc_sequence seq = {
		.buffer      = &sample,
		.buffer_size = sizeof(sample),
	};
	adc_sequence_init_dt(&adc_chan, &seq);

	int32_t raw_sum = 0;
	int good = 0;

	for (int i = 0; i < 16; i++) {
		if (adc_read_dt(&adc_chan, &seq) == 0) {
			raw_sum += sample;
			good++;
		}
	}

	if (good == 0) {
		return -EIO;
	}

	int32_t mv = raw_sum / good;
	adc_raw_to_millivolts_dt(&adc_chan, &mv);
	/* LMT85 at 3.3V supply: T(°C) = (1822 - V_mV) / 10.88 */
	int32_t temp_milli_c = (1822 - mv) * 10000 / 1088 * 10;

	out->val1 = temp_milli_c / 1000;
	out->val2 = (temp_milli_c % 1000) * 1000;
	return 0;
}

static int handle_read(uint8_t reg, uint8_t *out)
{
	uint8_t bytes[sizeof(struct sensor_value)];

	memcpy(bytes, &temp_value, sizeof(struct sensor_value));
	if (reg < (uint8_t)sizeof(struct sensor_value)) {
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

	if (!adc_is_ready_dt(&adc_chan)) {
		printk("ADC not ready\n");
		return 0;
	}

	if (adc_channel_setup_dt(&adc_chan) < 0) {
		printk("ADC channel setup failed\n");
		return 0;
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

	while (1) {
		if (get_temp(&temp_value) == 0) {
			printk("T=%d.%06d C\n", temp_value.val1, temp_value.val2);
		} else {
			printk("ADC read failed\n");
		}
		k_sleep(K_MSEC(1000));
	}

	return 0;
}