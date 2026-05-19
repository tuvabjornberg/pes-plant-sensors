#ifndef HANDLER_INTERFACE_H
#define HANDLER_INTERFACE_H

#include <zephyr/device.h>
#include <stdint.h>

typedef int (*handler_read_fn)(uint8_t reg, uint8_t *out);
typedef int (*handler_write_fn)(uint8_t reg, uint8_t val);

typedef struct {
	handler_read_fn handle_read;
	handler_write_fn handle_write;
	uint8_t address;
} handler_config_t;

int register_target_with_handlers(const struct device *bus, const handler_config_t *cfg);

#endif /* HANDLER_INTERFACE_H */