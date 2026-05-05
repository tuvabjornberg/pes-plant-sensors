#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <string.h>

/* keep these in sync with the base station driver or things will break */
#define SN_START_BYTE    0xAA
#define SN_CMD_FETCH     0x01
#define SN_PAYLOAD_LEN   sizeof(int32_t)
#define SN_REPLY_LEN     (3 + SN_PAYLOAD_LEN + 1)
#define SN_REQUEST_LEN   3

static const struct device *const console   = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct device *const data_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t xor_checksum(const uint8_t *buf, size_t len)
{
	uint8_t cs = 0;

	for (size_t i = 0; i < len; i++) {
		cs ^= buf[i];
	}
	return cs;
}

static void send_reply(int32_t temp_milli_c)
{
	uint8_t pkt[SN_REPLY_LEN];

	pkt[0] = SN_START_BYTE;
	pkt[1] = SN_CMD_FETCH;
	pkt[2] = (uint8_t)SN_PAYLOAD_LEN;
	memcpy(&pkt[3], &temp_milli_c, sizeof(int32_t));
	pkt[SN_REPLY_LEN - 1] = xor_checksum(pkt, SN_REPLY_LEN - 1);

	for (int i = 0; i < SN_REPLY_LEN; i++) {
		uart_poll_out(data_uart, pkt[i]);
	}
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

	printk("Sensor node started\n");

	int32_t counter = 0;
	uint8_t req[SN_REQUEST_LEN];
	int rx_idx = 0;

	while (1) {
		uint8_t byte;

		if (uart_poll_in(data_uart, &byte) == 0) {
			req[rx_idx++] = byte;

			if (rx_idx == SN_REQUEST_LEN) {
				rx_idx = 0;

				if (req[0] == SN_START_BYTE &&
				    req[1] == SN_CMD_FETCH &&
				    req[2] == xor_checksum(req, 2)) {

					/* fake temp for now, swap in real ADC/I2C read later */
					int32_t temp = 20000 + (counter % 10) * 100;

					send_reply(temp);
					printk("replied: temp=%d mC  (req #%d)\n", temp, (int)counter);
					counter++;
				} else {
					/* garbage, throw away one byte and try again */
					memmove(req, req + 1, SN_REQUEST_LEN - 1);
					rx_idx = SN_REQUEST_LEN - 1;
				}
			}
		} else {
			k_sleep(K_USEC(100));
		}
	}

	return 0;
}
