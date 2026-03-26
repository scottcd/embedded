#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "app_logging.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_wifi app_wifi_t;
typedef app_wifi_t *app_wifi_handle_t;

typedef struct {
    const char *ssid;
    const char *password;
    int max_retry_count;
    int max_tx_power_quarter_dbm;
    app_logging_handle_t logger;
} app_wifi_config_t;

esp_err_t app_wifi_init(app_wifi_handle_t *handle, const app_wifi_config_t *config);
esp_err_t app_wifi_start(app_wifi_handle_t handle);
esp_err_t app_wifi_stop(app_wifi_handle_t handle);
esp_err_t app_wifi_deinit(app_wifi_handle_t *handle);

int app_wifi_is_connected(app_wifi_handle_t handle);
const char *app_wifi_ssid(app_wifi_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
