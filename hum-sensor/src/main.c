/**
 * @file main.c
 * @brief BME680 temperature and humidity sensor reader using the Bosch
 * SensorAPI.
 *
 * Reads temperature and humidity from a BME680 sensor over I2C using the
 * official Bosch BME68x SensorAPI. Results are printed over UART every 3
 * seconds.
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "bme68x.h"

/* I2C address of the BME680 - HIGH (0x77) when SDO is pulled high */
#define BME680_ADDR BME68X_I2C_ADDR_HIGH
/* Devicetree label for the I2C bus the sensor is connected to */
#define I2C_LABEL i2c0

/* Zephyr I2C bus device handle */
static const struct device* i2c_bus;

/* Bosch SensorAPI device descriptor */
static struct bme68x_dev bme;

/* Sensor I2C address passed to the API via intf_ptr */
static uint8_t dev_addr = BME68X_I2C_ADDR_HIGH;

/**
 * @brief I2C read callback for the Bosch SensorAPI.
 *
 * The SensorAPI calls this function whenever it needs to read registers
 * from the sensor. Performs a combined write (register address) + read
 * (register data) transaction over I2C.
 *
 * @param reg_addr  Register address to read from.
 * @param reg_data  Buffer to store the read data.
 * @param len       Number of bytes to read.
 * @param intf_ptr  Pointer to the I2C device address (uint8_t).
 * @return 0 on success, negative errno on failure.
 */
static BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t* reg_data,
                                            uint32_t len, void* intf_ptr) {
    uint8_t addr = *(uint8_t*)intf_ptr;
    return i2c_write_read(i2c_bus, addr, &reg_addr, 1, reg_data, len);
}

/**
 * @brief I2C write callback for the Bosch SensorAPI.
 *
 * The SensorAPI calls this function whenever it needs to write to registers
 * on the sensor. Prepends the register address to the data and performs a
 * single I2C write transaction.
 *
 * @param reg_addr  Register address to write to.
 * @param reg_data  Data to write.
 * @param len       Number of bytes to write.
 * @param intf_ptr  Pointer to the I2C device address (uint8_t).
 * @return 0 on success, negative errno on failure.
 */
static BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr,
                                             const uint8_t* reg_data,
                                             uint32_t len, void* intf_ptr) {
    uint8_t addr = *(uint8_t*)intf_ptr;
    /* Combine register address and data into a single buffer for one I2C
     * transaction */
    uint8_t buf[len + 1];
    buf[0] = reg_addr;
    memcpy(&buf[1], reg_data, len);
    return i2c_write(i2c_bus, buf, len + 1, addr);
}

/**
 * @brief Delay callback for the Bosch SensorAPI.
 *
 * The SensorAPI calls this to wait between operations, for example
 * after triggering a measurement. Uses Zephyr's kernel sleep.
 *
 * @param period    Delay duration in microseconds.
 * @param intf_ptr  Unused interface pointer.
 */
static void bme68x_delay_us(uint32_t period, void* intf_ptr) {
    k_sleep(K_USEC(period));
}

/**
 * @brief Application entry point.
 *
 * Initializes the BME680 sensor, configures oversampling settings,
 * then repeatedly triggers forced-mode measurements and prints
 * temperature and humidity over UART every 3 seconds.
 *
 * @return -1 on initialization failure, never returns on success.
 */
int main(void) {
    /* Get the I2C bus device from the devicetree */
    i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));
    if (i2c_bus == NULL) {
        printf("No device found; did initialization fail?\n");
        return -1;
    }

    /* Wire up the Bosch SensorAPI callbacks and interface settings */
    bme.read = bme68x_i2c_read;
    bme.write = bme68x_i2c_write;
    bme.delay_us = bme68x_delay_us;
    bme.intf = BME68X_I2C_INTF;
    bme.intf_ptr = &dev_addr;

    /* Initialize the sensor - reads chip ID and loads calibration data */
    if (bme68x_init(&bme) != BME68X_OK) {
        printf("Could not communicate with sensor.\n");
        return -1;
    }

    /* Configure oversampling rates and IIR filter */
    struct bme68x_conf conf;
    bme68x_get_conf(&conf, &bme);
    conf.os_hum = BME68X_OS_2X;    /* 2x oversampling for humidity */
    conf.os_temp = BME68X_OS_8X;   /* 8x oversampling for temperature */
    conf.os_pres = BME68X_OS_NONE; /* Pressure measurement disabled */
    conf.filter = BME68X_FILTER_OFF;
    bme68x_set_conf(&conf, &bme);

    /* Disable the gas heater since we only need temperature and humidity */
    struct bme68x_heatr_conf heatr_conf = {0};
    heatr_conf.enable = BME68X_DISABLE;
    bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);

    while (1) {
        /* Trigger a single forced-mode measurement */
        bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);

        /* Wait for the measurement to complete based on oversampling settings
         */
        uint32_t delay_us =
            bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme);
        k_sleep(K_USEC(delay_us));

        /* Retrieve the measurement result */
        struct bme68x_data data;
        uint8_t n_fields;
        bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);

        if (n_fields) {
            /* Split floats into whole and decimal parts for printf */
            int t_whole = (int)data.temperature;
            int t_dec = (int)((data.temperature - t_whole) * 100);
            int h_whole = (int)data.humidity;
            int h_dec = (int)((data.humidity - h_whole) * 100);

            printf("Temperature: %d.%02d C  Humidity: %d.%02d %%\n", t_whole,
                   t_dec, h_whole, h_dec);
        }

        /* Wait 3 seconds before the next measurement */
        k_sleep(K_SECONDS(3));
    }
}