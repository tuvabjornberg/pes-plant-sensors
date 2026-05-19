#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>

int moisture_sensor_init(const struct device *i2c_bus);
int moisture_sensor_read(uint16_t *moisture);

#endif