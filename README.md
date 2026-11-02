# ESP32 Weather Station

#### Video Demo:  <URL HERE>

A small Wi-Fi weather station built on an ESP32 microcontroller and DHT22 temperature and humidity sensor.
When the board powers on, it joins Wi-Fi network and starts an HTTP server.
Open the board's IP address in any browser on the same network and you get a live dashboard showing the current temperature and relative humidity, refreshed every two seconds.
VPN server can be set up on your router if you want to have access to the dashboard outside local Wi-Fi network, but this is out of scope of this repo
There is nothing to install on the client side and no cloud service involved. The ESP32 serves the page, takes the measurements and answers the requests.

The firmware is written in C on top of Espressif's ESP-IDF framework (v6.2) and FreeRTOS. It uses the `esp-idf-lib/dht` component to handle the sensor's single-wire timing protocol.

## Hardware

- An ESP32 S3 development board
- An DHT22 sensor (DHT22-compatible protocol), data line connected to **GPIO 4**
- Power for the sensor (3.3 V) and a common ground

The sensor type and pin are constants at the top of `main/sensor.c`, so you only need to change one line to use a DHT11, a DHT22 or a different GPIO.

## Building and flashing

```sh
idf.py set-target esp32s3
idf.py menuconfig          # Configuration -> WiFi SSID / WiFi Password
idf.py build flash monitor
```

When the device connects, the serial monitor prints a line such as `got ip:192.168.1.42`. Open that address in a browser to see the dashboard.

## How it works

At boot, `app_main` calls `wifi_init_sta()`. That function brings up NVS, the network interface and the default event loop, then starts the Wi-Fi driver in station mode.
Everything after that is event-driven.
When the station starts, it tries to connect to the access point.
When it receives an IP address, it starts the web server.
If the connection drops, it retries up to a configurable number of times and then shuts the web server down.

The server exposes two endpoints:

| Route       | Response                                                                                                                               |
| ----------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| `GET /`     | The dashboard (`index.html`) embedded in the firmware                                                                                  |
| `GET /data` | A fresh sensor reading as JSON, e.g. `{"temperature": 22.4, "humidity": 41.0}`, or `503 Service Unavailable` if the sensor read failed |

The dashboard's JavaScript calls `fetch("/data")` every two seconds and updates the page with the new values and the time of the last successful update.

## Files

**`CMakeLists.txt`** is the top-level ESP-IDF project file.
It turns on `MINIMAL_BUILD`, so only the `main` component and its real dependencies are compiled.
This keeps build times and binary size down.

**`main/CMakeLists.txt`** registers the `main` component.
It lists the four C sources and the IDF components they need (`esp_wifi`, `nvs_flash`, `esp_netif`, `esp_http_server`).
It also uses `EMBED_TXTFILES` to compile `index.html` directly into the firmware image.

**`main/idf_component.yml`** is the component manager manifest.
It pulls in `esp-idf-lib/dht`, and `dependencies.lock` pins the exact versions that were resolved.

**`main/Kconfig.projbuild`** defines the "Configuration" menu in `menuconfig`:
Wi-Fi SSID, password, maximum retry count, minimum accepted authentication mode, and the WPA3 SAE options.

**`main/main.c`** is the entry point.
It starts Wi-Fi and stays alive while the server is running.
All the real work happens in event handlers and in the HTTP server's own task.

**`main/wifi.c` / `wifi.h`** handle the Wi-Fi station.
The code has three separate event handlers: one for station start, one for disconnect, and one for "got IP".
The disconnect and got-IP handlers receive a pointer to the global server handle, so they can start the server when the network comes up and stop it when the network is lost.
`wifi_init_sta()` blocks on a FreeRTOS event group until it either connects or runs out of retries.

**`main/web_server.c` / `web_server.h`** contain the HTTP server.
They define the two URI handlers, `start_webserver()` and `stop_webserver()`, and the shared `server` handle.
The index handler sends the embedded HTML.
The readings handler takes a measurement and formats it as JSON with `asprintf`.

**`main/sensor.c` / `sensor.h`** wrap the DHT driver.
`read_dht()` stores the latest values in a global `SensorReadings` struct and returns `SENSOR_OK` or `SENSOR_ERR`.
It also logs either the reading or the error.

**`main/index.html`** is the single-page dashboard.
It has no external dependencies and all CSS and JavaScript are inline.

## Design decisions

**Embedding the HTML instead of using a filesystem.**
`EMBED_TXTFILES` places the page in flash as a constant byte array, and serving it is a single `httpd_resp_send` call.
The trade-off is that changing the page requires reflashing the firmware, which is acceptable for current scale

**Polling instead of WebSockets or Server-Sent Events.**
Both alternatives would push updates to the browser, but they need long-lived connections, extra server configuration and reconnection logic on the client.
The sensor can only be read about once every two seconds anyway, so polling `/data` at that interval gives the same freshness with a few lines of JavaScript.
I set `Connection: keep-alive` so the browser reuses its TCP connection instead of opening a new one for every poll.
I also set `lru_purge_enable`, so stale connections get recycled when the server runs out of sockets.

**Reading the sensor on demand.**
The alternative was a background FreeRTOS task that reads the sensor on a timer while the HTTP handler returns the cached value.
Reading inside the request handler means no extra task, no locking and no stale data.
The downside is that several browsers polling at once can query the sensor faster than it allows, and some of those requests will get a 503.
With one or two viewers on a home network, that has not been a problem.

**A clear failure response.**
When the sensor read fails, the endpoint returns `503 Service Unavailable` instead of zeros or the last value.
The dashboard then keeps showing the previous reading, and the "last update" time stops advancing, so you can see that the data is out of date.

**Tying the server's lifetime to the network.**
At first the server was started once at boot.
Moving start and stop into the IP and disconnect events means the server only runs while there is an address to serve on, and a reconnect brings it back automatically.

**Credentials in Kconfig.**
The SSID and password are set through `menuconfig` and saved in `sdkconfig`, which is in `.gitignore`.
That keeps them out of the source code and out of version control.
