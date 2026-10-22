// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP
// #include "esp_log.h"

#include "wifi.h"
#include "sensor.h"
// #include "web_server.h"


void app_main(void) {
  wifi_init_sta();

  // server = start_webserver();

  TickType_t last_wake = xTaskGetTickCount();
  while (1) {
    read_dht();
    xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(3000));
  }
}
