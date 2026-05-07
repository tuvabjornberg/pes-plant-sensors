#pragma once

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

void init_light_sensor(const struct device *i2c_bus);
void read_light_sensor(const struct device *i2c_bus); 