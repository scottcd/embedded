#include "app_logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct app_logging {
    const char *tag;
    app_log_level_t min_level;
    int use_color;
    int started;
};

static const char *app_logging_level_label(app_log_level_t level)
{
    switch (level) {
        case APP_LOG_LEVEL_DEBUG:
            return "DEBUG";
        case APP_LOG_LEVEL_INFO:
            return "INFO";
        case APP_LOG_LEVEL_WARNING:
            return "WARNING";
        case APP_LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "INFO";
    }
}

static const char *app_logging_level_color(app_log_level_t level)
{
    switch (level) {
        case APP_LOG_LEVEL_DEBUG:
            return "\033[36m";
        case APP_LOG_LEVEL_INFO:
            return "\033[32m";
        case APP_LOG_LEVEL_WARNING:
            return "\033[33m";
        case APP_LOG_LEVEL_ERROR:
            return "\033[31m";
        default:
            return "\033[0m";
    }
}

esp_err_t app_logging_init(app_logging_handle_t *handle, const app_logging_config_t *config)
{
    app_logging_t *instance;

    if (handle == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return ESP_ERR_NO_MEM;
    }

    instance->tag = config->tag != NULL ? config->tag : "app";
    instance->min_level = config->min_level;
    instance->use_color = config->use_color;
    *handle = instance;
    return ESP_OK;
}

esp_err_t app_logging_start(app_logging_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    handle->started = 1;
    app_logging_infof(handle, "logging started");
    return ESP_OK;
}

esp_err_t app_logging_stop(app_logging_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    handle->started = 0;
    return ESP_OK;
}

esp_err_t app_logging_deinit(app_logging_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    free(*handle);
    *handle = NULL;
    return ESP_OK;
}

void app_logging_vlogf(app_logging_handle_t handle, app_log_level_t level, const char *format, va_list args)
{
    const char *label;
    const char *color;

    if (handle == NULL || format == NULL || !handle->started || level < handle->min_level) {
        return;
    }

    label = app_logging_level_label(level);
    color = app_logging_level_color(level);

    if (handle->use_color) {
        printf("%s[%-7s]\033[0m ", color, label);
    } else {
        printf("[%-7s] ", label);
    }

    vprintf(format, args);
    printf("\n");
}

void app_logging_logf(app_logging_handle_t handle, app_log_level_t level, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    app_logging_vlogf(handle, level, format, args);
    va_end(args);
}

void app_logging_debugf(app_logging_handle_t handle, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    app_logging_vlogf(handle, APP_LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void app_logging_infof(app_logging_handle_t handle, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    app_logging_vlogf(handle, APP_LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void app_logging_warningf(app_logging_handle_t handle, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    app_logging_vlogf(handle, APP_LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void app_logging_errorf(app_logging_handle_t handle, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    app_logging_vlogf(handle, APP_LOG_LEVEL_ERROR, format, args);
    va_end(args);
}
