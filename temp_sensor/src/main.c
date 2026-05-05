#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/usb/usb_device.h>

static const struct adc_dt_spec adc_chan =
	ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

int main(void)
{
	const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
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

	int16_t sample;
	struct adc_sequence seq = {
		.buffer = &sample,
		.buffer_size = sizeof(sample),
	};
	adc_sequence_init_dt(&adc_chan, &seq);

	printk("Temp sensor app started\n");

	const int N = 256;

	while (1) {
		int32_t raw_sum = 0;
		int good = 0;

		for (int i = 0; i < N; i++) {
			if (adc_read_dt(&adc_chan, &seq) == 0) {
				raw_sum += sample;
				good++;
			}
		}

		if (good == 0) {
			printk("ADC read failed\n");
		} else {
			int32_t raw_avg = raw_sum / good;
			int32_t mv = raw_avg;
			adc_raw_to_millivolts_dt(&adc_chan, &mv);
			/* LMT85 at 3.3V supply: T(°C) = (1822 - V_mV) / 10.88 */
			int32_t temp_c100 = (1822 - mv) * 10000 / 1088;
			printk("raw=%d  V=%d mV  T=%d.%02d C\n",
			       raw_avg, mv,
			       temp_c100 / 100,
			       (temp_c100 < 0 ? -temp_c100 : temp_c100) % 100);
		}

		k_sleep(K_MSEC(500));
	}

	return 0;
}
