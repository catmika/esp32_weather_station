#include "esp_log.h"
#include <esp_check.h>
#include <stdlib.h>

#include <esp_http_server.h>

static const char *TAG = "web server";

httpd_handle_t server = NULL;

void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server) {
    ESP_LOGI(TAG, "Stopping webserver");
    if (stop_webserver(*server) == ESP_OK) {
      *server = NULL;
    } else {
      ESP_LOGE(TAG, "Failed to stop http server");
    }
  }
}

static esp_err_t get_readings_handler(httpd_req_t *req) {
  char *buf;
  size_t buf_len;

  /* Get header value string length and allocate memory for length + 1,
   * extra byte for null termination */
  buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
  if (buf_len > 1) {
    buf = malloc(buf_len);
    ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
    /* Copy null terminated value string into buffer */
    if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
      ESP_LOGI(TAG, "Found header => Host: %s", buf);
    }
    free(buf);
  }

  /* Set some custom headers */
  // httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");

  /* Send response with custom headers and body set as the
   * string passed in user context*/
  const char *resp_str = "Hello ESP";
  httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

static const httpd_uri_t get_readings = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_readings_handler,
};

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
    // #if CONFIG_EXAMPLE_ENABLE_SSE_HANDLER
    //     httpd_register_uri_handler(server, &sse); // Register SSE handler
    // #endif
    // #if CONFIG_EXAMPLE_BASIC_AUTH
    //     httpd_register_basic_auth(server);
    // #endif
    return server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}

esp_err_t stop_webserver(httpd_handle_t server) {
  // Stop the httpd server
  return httpd_stop(server);
}
