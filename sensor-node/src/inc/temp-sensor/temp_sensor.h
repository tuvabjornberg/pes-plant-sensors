#include <zephyr/drivers/adc.h>

int init_temp_sensor(const struct adc_dt_spec *adc_chan);
int read_temp_sensor(int32_t *temp_milli_c);
