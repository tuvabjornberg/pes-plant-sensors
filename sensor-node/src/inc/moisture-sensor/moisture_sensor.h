#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>

int init_moisture_sensor(const struct device *i2c_bus);
int read_moisture_sensor(uint16_t *moisture);

#endif