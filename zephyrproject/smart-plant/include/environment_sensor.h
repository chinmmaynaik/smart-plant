#ifndef ENVIRONMENT_SENSOR_H
#define ENVIRONMENT_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double temperature_c;
    double humidity_per;
    double pressure_kpa;
    int64_t timestamp; // Unix timestamp in milliseconds
    bool valid;
}environment_data_t;

int environment_sensor_init(void);
int environment_sensor_read(environment_data_t *data);

#endif // ENVIRONMENT_SENSOR_H