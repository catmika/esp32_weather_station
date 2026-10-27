idf_component_register(SRCS "main.c" "wifi.c" "sensor.c" "web_server.c"
                       PRIV_REQUIRES esp_wifi nvs_flash esp-tls esp_http_server esp_netif
                       INCLUDE_DIRS "")
