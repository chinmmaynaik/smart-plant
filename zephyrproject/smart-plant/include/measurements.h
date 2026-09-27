#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "environment_sensor.h"
#include "soil_sensor.h"

typedef struct {
    environment_data_t environment;
    soil_sensor_data_t soil;
}plant_measurement_t;

#endif