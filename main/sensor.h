#pragma once

typedef int sensor_err_t;

#define SENSOR_OK 0
#define SENSOR_ERR (-1)

sensor_err_t read_dht(void);

typedef struct {
  float temperature;
  float humidity;
} SensorReadings;

extern SensorReadings sensor_readings;
