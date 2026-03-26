#ifndef APP_WEB_H
#define APP_WEB_H

#include <stddef.h>
#include <stdint.h>

#include "app_logging.h"
#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_web app_web_t;
typedef app_web_t *app_web_handle_t;
typedef esp_err_t (*app_web_handler_t)(httpd_req_t *request, void *user_ctx);

typedef struct {
    const char *uri;
    const char *content_type;
    const char *body;
    app_web_handler_t handler;
    void *user_ctx;
} app_web_page_t;

typedef struct {
    app_logging_handle_t logger;
    const char *app_name;
    uint16_t port;
    const app_web_page_t *pages;
    size_t page_count;
} app_web_config_t;

esp_err_t app_web_init(app_web_handle_t *handle, const app_web_config_t *config);
esp_err_t app_web_start(app_web_handle_t handle);
esp_err_t app_web_stop(app_web_handle_t handle);
esp_err_t app_web_deinit(app_web_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
