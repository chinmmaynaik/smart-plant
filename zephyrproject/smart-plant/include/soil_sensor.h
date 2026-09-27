#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t raw_adc;
    uint32_t moisture_percentage;
    uint32_t filtered_adc;
    int64_t timestamp; // Unix timestamp in milliseconds
    bool valid;
}soil_sensor_data_t;

int soil_sensor_init(void);
int soil_sensor_read(soil_sensor_data_t *data);
int soil_sensor_calibrate(uint32_t dry_adc, uint32_t wet_adc);

#endif // SOIL_SENSOR_H