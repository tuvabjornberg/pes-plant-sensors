#pragma once
#include "humidity-sensor.h"
#include "bme680_reg.h"
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>


void init_humidity_sensor(const struct device* i2c_bus) ;
void read_humidity_sensor(const struct device* i2c_bus);