#pragma once

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include "bme68x.h"

void bme_config(const struct device *i2c_bus);
void read_humidity(const struct device *i2c_bus); 