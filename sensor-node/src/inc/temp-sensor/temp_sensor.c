#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>

static const struct adc_dt_spec *adc_chan;
int init_temp_sensor(const struct adc_dt_spec *adc) {
    adc_chan = adc;
    return 0;
}

int read_temp_sensor(int32_t *temp_milli_c) {
    int16_t sample;
    struct adc_sequence seq = {
        .buffer = &sample,
        .buffer_size = sizeof(sample),
    };
    adc_sequence_init_dt(adc_chan, &seq);

    int32_t raw_sum = 0;
    int good = 0;

    for (int i = 0; i < 16; i++) {
        if (adc_read_dt(adc_chan, &seq) == 0) {
            raw_sum += sample;
            good++;
        }
    }

    if (good == 0) {
        return -EIO;
    }

    int32_t mv = raw_sum / good;
    adc_raw_to_millivolts_dt(adc_chan, &mv);
    /* LMT85 at 3.3V supply: T(°C) = (1822 - V_mV) / 10.88 */
    *temp_milli_c = (1822 - mv) * 10000 / 1088 * 10;

    return 0;
}
