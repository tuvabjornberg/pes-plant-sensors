#include "light_sensor.h"

#define LTR303_ADDR 0x29

#define ALS_CONTR     0x80
#define ALS_MEAS_RATE 0x85

#define ALS_DATA_CH1_0 0x88
#define ALS_DATA_CH1_1 0x89
#define ALS_DATA_CH0_0 0x8A
#define ALS_DATA_CH0_1 0x8B

#define ALS_STATUS 0x8C

#define ALS_CONTR_STANDBY 0x00
#define ALS_CONTR_ACTIVE  0x01
#define ALS_MEAS_RATE_VAL 0x12

static const struct device *i2c_bus_ref;

static uint16_t ch0_data;
static uint16_t ch1_data;

int init_light_sensor(const struct device *i2c_bus) {
    i2c_bus_ref = i2c_bus;
    int res = i2c_reg_write_byte(i2c_bus, LTR303_ADDR, ALS_CONTR, ALS_CONTR_STANDBY);
    if (res < 0) {
        return res;
    }
    return i2c_reg_write_byte(i2c_bus, LTR303_ADDR, ALS_MEAS_RATE, ALS_MEAS_RATE_VAL);
}

float calculate_lux() {
    float lux = 0.0f;
    if ((ch0_data + ch1_data) != 0) {
        float ratio = (float)ch1_data / (ch0_data + ch1_data);
        float ALS_GAIN = 1.0f;
        float ALS_INT = 1.0f;

        if (ratio < 0.45f) {
            lux = (1.7743f * ch0_data + 1.1059f * ch1_data) / (ALS_GAIN * ALS_INT);
        } else if (ratio < 0.64f) {
            lux = (4.2785f * ch0_data - 1.9548f * ch1_data) / (ALS_GAIN * ALS_INT);
        } else if (ratio < 0.85f) {
            lux = (0.5926f * ch0_data + 0.1185f * ch1_data) / (ALS_GAIN * ALS_INT);
        } else {
            lux = 0.0f;
        }
    }

    return lux;
}

int read_light_sensor(float *lux) {
    i2c_reg_write_byte(i2c_bus_ref, LTR303_ADDR, ALS_CONTR, ALS_CONTR_ACTIVE);
    k_msleep(500);

    uint8_t status;
    i2c_reg_read_byte(i2c_bus_ref, LTR303_ADDR, ALS_STATUS, &status);

    if (status & 0x04) {
        uint8_t lsb, msb;

        i2c_reg_read_byte(i2c_bus_ref, LTR303_ADDR, ALS_DATA_CH1_0, &lsb);
        i2c_reg_read_byte(i2c_bus_ref, LTR303_ADDR, ALS_DATA_CH1_1, &msb);
        ch1_data = (msb << 8) | lsb;

        i2c_reg_read_byte(i2c_bus_ref, LTR303_ADDR, ALS_DATA_CH0_0, &lsb);
        i2c_reg_read_byte(i2c_bus_ref, LTR303_ADDR, ALS_DATA_CH0_1, &msb);
        ch0_data = (msb << 8) | lsb;

        *lux = calculate_lux();
    }

    i2c_reg_write_byte(i2c_bus_ref, LTR303_ADDR, ALS_CONTR, ALS_CONTR_STANDBY);

    return 0;
}