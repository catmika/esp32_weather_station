// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "web_server.h"
#include "wifi.h"

void app_main(void) {
  wifi_init_sta();

  while (server) {
    sleep(5);
  }
}
