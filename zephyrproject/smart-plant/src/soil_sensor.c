#include "soil_sensor.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <stdint.h>

// ADC channel described in app.overlay file
#define ADC_NODE DT_PATH(zephyr_user)

// number of samples in each measurement batch
#define SAMPLE_COUNT 8

// smoothing factor for the exponential moving average filter
#define FILTER_ALPHA 4

static int32_t filtered_value =0;
static bool filter_initialized = false;

uint32_t calibration_dry = 3000;
uint32_t calibration_wet = 1500;

static const struct adc_dt_spec soil_adc = ADC_DT_SPEC_GET_BY_IDX(ADC_NODE, 0);

void soil_sensor_set_calibration(uint32_t dry_raw, uint32_t wet_raw) {
    
    if(dry_raw < 0 || dry_raw > 4095 ||
        wet_raw < 0 || wet_raw > 4095 || dry_raw == wet_raw){
            return;
    }

    calibration_dry = dry_raw;
    calibration_wet = wet_raw;

    filter_initialized = false;
}

static int32_t moisture_from_raw(int32_t raw)
{
    int32_t numerator =
        (raw - calibration_dry) * 100;

    int32_t denominator =
        calibration_wet - calibration_dry;

    int32_t percent = numerator / denominator;

    if (percent < 0) {
        percent = 0;
    }

    if (percent > 100) {
        percent = 100;
    }

    return percent;
}
int soil_sensor_init(void) {

    int ret;

    if (!adc_is_ready_dt(&soil_adc)) {
    return -ENODEV;
    }

    ret = adc_channel_setup_dt(&soil_adc);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int soil_sensor_read(soil_sensor_data_t *data) {

    int ret;
    uint32_t sum = 0;

    if(data == NULL){
        return -EINVAL;
    }

    data->valid = false; // Mark data as invalid initially
 
    if (!adc_is_ready_dt(&soil_adc)) {
    return -ENODEV;
    }

    for(int i = 0; i < SAMPLE_COUNT; i++){

        int16_t sample;
        

        struct adc_sequence sequence = {
            .buffer = &sample,
            .buffer_size = sizeof(sample)
        };
    
        ret = adc_sequence_init_dt(&soil_adc, &sequence);

        if (ret < 0) {
            printk("ADC sequence initialization failed: %d\n", ret);
            return ret;
        }

        ret = adc_read_dt(&soil_adc, &sequence);

        if (ret < 0) {
            printk("ADC read failed: %d\n", ret);
            return ret;
        }

        /* Print the actual ADC result. */
        // printk("ADC DEBUG: sample = %d\n", sample);

        if (sample < 0 || sample > 4095) {
            printk("ADC value out of range: %d\n", sample);
            return -ERANGE;
        }

        sum += sample;
    }

    int32_t raw = sum / SAMPLE_COUNT;

    if(!filter_initialized){
        filtered_value = raw;
        filter_initialized = true;
    }else{
        filtered_value += (raw - filtered_value)/FILTER_ALPHA;
    }

    data->filtered_adc = filtered_value;
    printk("adc filtered value: %d\n", filtered_value);
    data->moisture_percentage = moisture_from_raw(filtered_value);
    data->raw_adc = raw;
    data->timestamp = k_uptime_get(); // Get the current time in milliseconds
    data->valid = true;

    return 0;
}