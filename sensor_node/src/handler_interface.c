#include <zephyr/drivers/i2c.h>
#include "handler_interface.h"

static const handler_config_t *registered_config;
static struct i2c_target_config target_config;

typedef enum {
	COMM_IDLE,
	COMM_WRITE_REQUESTED,
	COMM_ADDR_RECEIVED,
} comm_state_t;

static struct {
	comm_state_t state;
	uint8_t addr;
} comm;

static int write_requested_cb(struct i2c_target_config *config)
{
	comm.state = COMM_WRITE_REQUESTED;
	return 0;
}

static int write_received_cb(struct i2c_target_config *config, uint8_t val)
{
	if (comm.state == COMM_WRITE_REQUESTED) {
		comm.addr = val;
		comm.state = COMM_ADDR_RECEIVED;
	} else {
		registered_config->handle_write(comm.addr, val);
		comm.addr++;
	}
	return 0;
}

static int read_requested_cb(struct i2c_target_config *config, uint8_t *out)
{
	if (comm.state == COMM_ADDR_RECEIVED) {
		registered_config->handle_read(comm.addr, out);
	} else {
		*out = 0;
	}
	return 0;
}

static int read_processed_cb(struct i2c_target_config *config, uint8_t *val)
{
	comm.addr++;
	registered_config->handle_read(comm.addr, val);
	return 0;
}

static int stop_cb(struct i2c_target_config *config)
{
	comm.state = COMM_IDLE;
	return 0;
}

static const struct i2c_target_callbacks handler_callbacks = {
	.write_requested = write_requested_cb,
	.read_requested  = read_requested_cb,
	.write_received  = write_received_cb,
	.read_processed  = read_processed_cb,
	.stop            = stop_cb,
};

int register_target_with_handlers(const struct device *bus, const handler_config_t *cfg)
{
	registered_config = cfg;
	target_config.address = cfg->address;
	target_config.callbacks = &handler_callbacks;
	return i2c_target_register(bus, &target_config);
}