#define DT_DRV_COMPAT pes_sensor_node

#define LOG_LEVEL CONFIG_SENSOR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_node);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#include "sensor_node.h"

#define REPLY_TIMEOUT_MS 500

struct sensor_node_config {
	const struct device *uart;
};

struct sensor_node_data {
	int32_t temp_milli_c;
};

static uint8_t xor_checksum(const uint8_t *buf, size_t len)
{
	uint8_t cs = 0;

	for (size_t i = 0; i < len; i++) {
		cs ^= buf[i];
	}
	return cs;
}

static int uart_read_byte(const struct device *uart, uint8_t *out)
{
	int64_t deadline = k_uptime_get() + REPLY_TIMEOUT_MS;

	while (uart_poll_in(uart, out) != 0) {
		if (k_uptime_get() >= deadline) {
			return -ETIMEDOUT;
		}
		k_sleep(K_USEC(200));
	}
	return 0;
}

static int sensor_node_sample_fetch(const struct device *dev,
				    enum sensor_channel chan)
{
	const struct sensor_node_config *cfg = dev->config;
	struct sensor_node_data *data = dev->data;

	uint8_t req[SN_REQUEST_LEN] = {SN_START_BYTE, SN_CMD_FETCH, 0};

	req[2] = xor_checksum(req, 2);
	for (int i = 0; i < SN_REQUEST_LEN; i++) {
		uart_poll_out(cfg->uart, req[i]);
	}

	uint8_t pkt[SN_REPLY_LEN];

	for (int i = 0; i < SN_REPLY_LEN; i++) {
		if (uart_read_byte(cfg->uart, &pkt[i]) != 0) {
			LOG_ERR("timeout waiting for reply");
			return -ETIMEDOUT;
		}
		if (i == 0 && pkt[0] != SN_START_BYTE) {
			i = -1; /* resync */
		}
	}

	if (xor_checksum(pkt, SN_REPLY_LEN - 1) != pkt[SN_REPLY_LEN - 1]) {
		LOG_ERR("bad checksum");
		return -EIO;
	}

	memcpy(&data->temp_milli_c, &pkt[3], sizeof(int32_t));
	return 0;
}

static int sensor_node_channel_get(const struct device *dev,
				   enum sensor_channel chan,
				   struct sensor_value *val)
{
	struct sensor_node_data *data = dev->data;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	val->val1 = data->temp_milli_c / 1000;
	val->val2 = (data->temp_milli_c % 1000) * 1000;
	return 0;
}

static const struct sensor_driver_api sensor_node_api = {
	.sample_fetch = sensor_node_sample_fetch,
	.channel_get  = sensor_node_channel_get,
};

static int sensor_node_init(const struct device *dev)
{
	const struct sensor_node_config *cfg = dev->config;

	if (!device_is_ready(cfg->uart)) {
		LOG_ERR("UART not ready");
		return -ENODEV;
	}

	return 0;
}

static struct sensor_node_data sensor_node_data;
static const struct sensor_node_config sensor_node_config = {
	.uart = DEVICE_DT_GET(DT_INST_PHANDLE(0, uart)),
};

SENSOR_DEVICE_DT_INST_DEFINE(0,
	sensor_node_init,
	NULL,
	&sensor_node_data,
	&sensor_node_config,
	POST_KERNEL,
	CONFIG_SENSOR_INIT_PRIORITY,
	&sensor_node_api);
