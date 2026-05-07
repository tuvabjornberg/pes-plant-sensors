#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>

int moisture_sensor_init(void);
int moisture_sensor_read(uint16_t *moisture);

#endif