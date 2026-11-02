#include "esp_log.h"
#include "soc/gpio_num.h"

#include "dht.h"
#include "sensor.h"

static const char *TAG = "dht";

static const dht_sensor_type_t dht_type = DHT_TYPE_AM2301;
static const gpio_num_t dht_pin = GPIO_NUM_4;

SensorReadings sensor_readings = {0.0, 0.0};

sensor_err_t read_dht(void) {
  esp_err_t res =
      dht_read_float_data(dht_type, dht_pin, &sensor_readings.humidity,
                          &sensor_readings.temperature);

  if (res != ESP_OK) {
    ESP_LOGW(TAG,
             "Failed to read from DHT sensor, pin: %d, type: %d, error: %s",
             dht_pin, dht_type, esp_err_to_name(res));
    return SENSOR_ERR;
  } else {
    ESP_LOGI(TAG, "Temperature: %.1f, Humidity: %.1f",
             sensor_readings.temperature, sensor_readings.humidity);
    return SENSOR_OK;
  }
}
