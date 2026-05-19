#pragma once

#include "bme68x.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

void init_humidity_sensor(const struct device *i2c_bus);
int read_humidity_sensor(float *humidity);