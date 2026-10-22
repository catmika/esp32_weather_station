#include "esp_log.h"
#include "soc/gpio_num.h"

#include "dht.h"

static const char *TAG = "dht";

static const dht_sensor_type_t dht_type = DHT_TYPE_AM2301;
static const gpio_num_t dht_pin = GPIO_NUM_4;
static float hum = 0.0;
static float temp = 0.0;

void read_dht(void) {
  esp_err_t res = dht_read_float_data(dht_type, dht_pin, &hum, &temp);

  if (res != ESP_OK) {
    ESP_LOGW(TAG,
             "Failed to read from DHT sensor, pin: %s, type: %s, error: %d",
             dht_pin, dht_type, res);
  } else {
    ESP_LOGI(TAG, "Temperature: %.1f, Humidity: %.1f", temp, hum);
  }
}
