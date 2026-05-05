
#include <zephyr/kernel.h>

typedef int (*handle_reg_read_t)(uint8_t reg, uint8_t *out);
typedef int (*handle_reg_write_t)(uint8_t reg, uint8_t out);

typedef struct handler_config {
    uint16_t address;
    handle_reg_read_t handle_read;
    handle_reg_write_t handle_write;
} handler_config_t;

int register_target_with_handlers(const struct device *dev, handler_config_t *cfg);