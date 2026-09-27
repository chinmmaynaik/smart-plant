#include "environment_sensor.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define BME280_NODE DT_ALIAS(environment_sensor)

// get the device structure from the device tree
static const struct device *bme_dev = DEVICE_DT_GET(BME280_NODE);

#if !DT_NODE_EXISTS(BME280_NODE)
#error "Environmental sensor alias is missing"
#endif

int environment_sensor_init(void) {
    if (!device_is_ready(bme_dev)) {
        return -ENODEV;
    }
    return 0;
}

int environment_sensor_read(environment_data_t *data) {

    struct sensor_value temperature, humidity, pressure;
    int ret;

    if (!device_is_ready(bme_dev)) {
        return -ENODEV;
    }
    
    ret = sensor_sample_fetch(bme_dev);
    if (ret < 0) {
        return ret;
    }

    ret = sensor_channel_get(bme_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
    if (ret < 0) {
        return ret;
    }

    ret = sensor_channel_get(bme_dev, SENSOR_CHAN_HUMIDITY, &humidity);
    if (ret < 0) {
        return ret;
    }

    ret = sensor_channel_get(bme_dev, SENSOR_CHAN_PRESS, &pressure);
    if (ret < 0) {
        return ret;
    }

    data->temperature_c = sensor_value_to_double(&temperature);
    data->humidity_per = sensor_value_to_double(&humidity);
    data->pressure_kpa = sensor_value_to_double(&pressure);

    if(!isfinite(data->temperature_c) || !isfinite(data->humidity_per) ||
         !isfinite(data->pressure_kpa) || data->temperature_c < -40 || 
        data->temperature_c > 85 || data->humidity_per < 0 || data->humidity_per > 100 ||
        data->pressure_kpa < 30 || data->pressure_kpa > 110) {
            return -ERANGE;
    }

    data->timestamp = k_uptime_get();
    data->valid = true;

    return 0;
}