#ifndef APP_LOGGING_H
#define APP_LOGGING_H

#include <stdarg.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_logging app_logging_t;
typedef app_logging_t *app_logging_handle_t;

typedef enum {
    APP_LOG_LEVEL_DEBUG = 0,
    APP_LOG_LEVEL_INFO = 1,
    APP_LOG_LEVEL_WARNING = 2,
    APP_LOG_LEVEL_ERROR = 3,
} app_log_level_t;

typedef struct {
    const char *tag;
    app_log_level_t min_level;
    int use_color;
} app_logging_config_t;

esp_err_t app_logging_init(app_logging_handle_t *handle, const app_logging_config_t *config);
esp_err_t app_logging_start(app_logging_handle_t handle);
esp_err_t app_logging_stop(app_logging_handle_t handle);
esp_err_t app_logging_deinit(app_logging_handle_t *handle);

void app_logging_logf(app_logging_handle_t handle, app_log_level_t level, const char *format, ...);
void app_logging_vlogf(app_logging_handle_t handle, app_log_level_t level, const char *format, va_list args);

void app_logging_debugf(app_logging_handle_t handle, const char *format, ...);
void app_logging_infof(app_logging_handle_t handle, const char *format, ...);
void app_logging_warningf(app_logging_handle_t handle, const char *format, ...);
void app_logging_errorf(app_logging_handle_t handle, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
