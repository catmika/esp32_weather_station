#include "esp_log.h"
#include <esp_check.h>
#include <stdlib.h>

#include <esp_http_server.h>

#include "sensor.h"

#define HTTPD_503 "503 Service Unavailable"

static const char *TAG = "web server";

httpd_handle_t server = NULL;

static esp_err_t get_readings_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Connection", "keep-alive");
  char *resp = NULL;

  sensor_err_t sensor_err = read_dht();

  if (sensor_err != SENSOR_OK) {
    httpd_resp_set_status(req, HTTPD_503);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  } else {
    int rc = asprintf(&resp,
                      "{\"temperature\": \"%.1f\","
                      "\"humidity\": \"%.1f\"}",
                      sensor_readings.temperature, sensor_readings.humidity);
    if (rc < 0) {
      ESP_LOGE(TAG, "asprintf() returned: %d", rc);
      return ESP_FAIL;
    }
  }

  httpd_resp_set_status(req, HTTPD_200);
  httpd_resp_send(req, resp, strlen(resp));
  free(resp);

  return ESP_OK;
}

static const httpd_uri_t get_readings = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_readings_handler,
};

esp_err_t stop_webserver(httpd_handle_t server) { return httpd_stop(server); }

httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &get_readings);
    return server;
  }

  ESP_LOGI(TAG, "Error starting server");
  return NULL;
}
