#include "humidity-sensor.h"

#define BME680_ADDR 0x77

#define PAR_T1_LSB 0xE9
#define PAR_T1_MSB 0xEA
#define PAR_T2_LSB 0x8A
#define PAR_T2_MSB 0x8B
#define PAR_T3_LSB 0x8C

#define PAR_H1_MSB 0xE3
#define PAR_H1_LSB 0xE2
#define PAR_H2_MSB 0xE1
#define PAR_H2_LSB 0xE2
#define PAR_H3 0xE4
#define PAR_H4 0xE5
#define PAR_H5 0xE6
#define PAR_H6 0xE7
#define PAR_H7 0xE8

#define OSRS_T 0b011 << 5
#define OSRS_H 0b010
#define OSRS_P 0b000 << 2
#define MODE 0b01

#define CTRL_MEAS_VAL OSRS_T | OSRS_P | MODE
#define CTRL_HUM_VAL OSRS_H

static int32_t adc_temp;
static int32_t adc_hum;
static float temp_final;
static float hum_final;
static int32_t t_fine;

static uint16_t par_t1;
static int16_t par_t2;
static int8_t par_t3;

static uint16_t par_h1;
static uint16_t par_h2;
static int8_t par_h3;
static int8_t par_h4;
static int8_t par_h5;
static uint8_t par_h6;
static int8_t par_h7;

static const struct device* i2c_bus_shared;

void read_calibration(void) {
    uint8_t lsb, msb, tmp;

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_T1_LSB, &lsb);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_T1_MSB, &msb);
    par_t1 = (msb << 8) | lsb;

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_T2_LSB, &lsb);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_T2_MSB, &msb);
    par_t2 = (msb << 8) | lsb;

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_T3_LSB, (uint8_t*)&par_t3);

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H1_MSB, &msb);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H1_LSB, &tmp);
    par_h1 = (msb << 4) | (tmp & 0x0F);

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H2_MSB, &msb);
    par_h2 = (msb << 4) | (tmp >> 4);

    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H3, (uint8_t*)&par_h3);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H4, (uint8_t*)&par_h4);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H5, (uint8_t*)&par_h5);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H6, &par_h6);
    i2c_reg_read_byte(i2c_bus_shared, BME680_ADDR, PAR_H7, (uint8_t*)&par_h7);
}

void convert_temp(void) {
    int32_t var1, var2, temp_comp;

    var1 = ((((adc_temp >> 3) - ((int32_t)par_t1 << 1))) * ((int32_t)par_t2)) >>
           11;
    var2 = (((((adc_temp >> 4) - ((int32_t)par_t1)) *
              ((adc_temp >> 4) - ((int32_t)par_t1))) >>
             12) *
            ((int32_t)par_t3 << 4)) >>
           14;

    t_fine = var1 + var2;
    temp_comp = ((t_fine * 5) + 128) >> 8;
    temp_final = temp_comp / 100.0f;
}

void convert_hum(void) {
    int32_t var1, var2, var3, var4, var5, var6;

    int32_t temp_scaled = (((int32_t)t_fine * 5) + 128) >> 8;

    var1 = adc_hum - ((int32_t)par_h1 << 4) -
           (((temp_scaled * (int32_t)par_h3) / 100) >> 1);

    var2 = ((int32_t)par_h2 *
            (((temp_scaled * (int32_t)par_h4) / 100) +
             (((temp_scaled * ((temp_scaled * (int32_t)par_h5) / 100)) >> 6) /
              100) +
             (1 << 14))) >>
           10;

    var3 = var1 * var2;
    var4 = (int32_t)par_h6 << 7;
    var4 = (var4 + ((temp_scaled * (int32_t)par_h7) / 100)) >> 4;
    var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
    var6 = (var4 * var5) >> 1;

    int32_t hum_comp = (((var3 + var6) >> 10) * 1000) >> 12;

    if (hum_comp > 100000) hum_comp = 100000;
    if (hum_comp < 0) hum_comp = 0;

    hum_final = hum_comp / 1000.0f;
}

void init_humidity_sensor(const struct device* i2c_bus) {
    i2c_bus_shared = i2c_bus;
    read_calibration();
    i2c_reg_write_byte(i2c_bus, BME680_ADDR, BME680_CTRL_HUM, CTRL_HUM_VAL);
    i2c_reg_write_byte(i2c_bus, BME680_ADDR, BME680_CTRL_MEAS, CTRL_MEAS_VAL);
    k_sleep(K_MSEC(50));
}

void read_humidity_sensor(const struct device* i2c_bus) {

    uint8_t tmsb, tlsb, txlsb;
    uint8_t hmsb, hlsb;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_MSB, &tmsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_LSB, &tlsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_XLSB, &txlsb);

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_HUM_MSB, &hmsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_HUM_LSB, &hlsb);

    adc_temp = ((int32_t)tmsb << 12) | ((int32_t)tlsb << 4) |
                ((int32_t)txlsb >> 4);
    adc_hum = ((int32_t)hmsb << 8) | hlsb;

    convert_temp();
    convert_hum();

    int h_whole = (int)hum_final;
    int h_dec = (int)((hum_final - h_whole) * 100);

    printk("Humidity: %d.%02d %%\n", h_whole, h_dec);
}