#ifndef APP_DHT22_H
#define APP_DHT22_H

#include <stdint.h>

#include "app_logging.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_dht22 app_dht22_t;
typedef app_dht22_t *app_dht22_handle_t;

typedef struct {
    gpio_num_t data_gpio;
    uint32_t minimum_read_interval_ms;
    app_logging_handle_t logger;
} app_dht22_config_t;

typedef struct {
    float temperature_c;
    float humidity_percent;
} app_dht22_reading_t;

esp_err_t app_dht22_init(app_dht22_handle_t *handle, const app_dht22_config_t *config);
esp_err_t app_dht22_start(app_dht22_handle_t handle);
esp_err_t app_dht22_stop(app_dht22_handle_t handle);
esp_err_t app_dht22_deinit(app_dht22_handle_t *handle);

esp_err_t app_dht22_read(app_dht22_handle_t handle, app_dht22_reading_t *reading);

#ifdef __cplusplus
}
#endif

#endif
