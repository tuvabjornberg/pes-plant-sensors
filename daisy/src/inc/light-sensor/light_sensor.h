#pragma once

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

void init_light_sensor(const struct device *i2c_bus);
int read_light_sensor(int *whole, int *decimal);
