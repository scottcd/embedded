#include "app_web.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"

struct app_web {
    app_logging_handle_t logger;
    const char *app_name;
    uint16_t port;
    const app_web_page_t *pages;
    size_t page_count;
    int started;
    httpd_handle_t server;
    app_web_page_t default_page;
    char default_html[1024];
};

static esp_err_t app_web_page_handler(httpd_req_t *request)
{
    const app_web_page_t *page = request->user_ctx;

    if (page == NULL) {
        return httpd_resp_send_500(request);
    }

    if (page->handler != NULL) {
        return page->handler(request, page->user_ctx);
    }

    if (page->body == NULL) {
        return httpd_resp_send_500(request);
    }

    httpd_resp_set_type(request, page->content_type != NULL ? page->content_type : "text/html");
    return httpd_resp_send(request, page->body, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t app_web_register_page(httpd_handle_t server, const app_web_page_t *page)
{
    httpd_uri_t uri = {
        .uri = page->uri,
        .method = HTTP_GET,
        .handler = app_web_page_handler,
        .user_ctx = (void *)page,
    };

    return httpd_register_uri_handler(server, &uri);
}

esp_err_t app_web_init(app_web_handle_t *handle, const app_web_config_t *config)
{
    app_web_t *instance;
    int html_len;

    if (handle == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return ESP_ERR_NO_MEM;
    }

    instance->logger = config->logger;
    instance->app_name = config->app_name != NULL ? config->app_name : "embedded app";
    instance->port = config->port != 0 ? config->port : 80;
    instance->pages = config->pages;
    instance->page_count = config->page_count;

    html_len = snprintf(
        instance->default_html,
        sizeof(instance->default_html),
        "<!doctype html>"
        "<html><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>%s</title>"
        "<style>"
        "body{font-family:ui-sans-serif,system-ui,sans-serif;background:#f4efe5;color:#1f1f1f;margin:0;padding:2rem;}"
        "main{max-width:42rem;margin:0 auto;background:#fffdf8;border:1px solid #d9cbb7;border-radius:16px;padding:2rem;box-shadow:0 18px 50px rgba(0,0,0,0.08);}"
        "h1{margin-top:0;font-size:2rem;}p{line-height:1.6;color:#4b443c;}code{background:#f1e7d7;padding:0.15rem 0.4rem;border-radius:6px;}"
        "</style></head><body><main><h1>%s</h1><p>HTTP server is running.</p>"
        "<p>Customize this page by passing <code>app_web_page_t</code> entries from your app.</p>"
        "</main></body></html>",
        instance->app_name,
        instance->app_name
    );
    if (html_len < 0 || (size_t)html_len >= sizeof(instance->default_html)) {
        free(instance);
        return ESP_ERR_INVALID_SIZE;
    }

    instance->default_page.uri = "/";
    instance->default_page.content_type = "text/html";
    instance->default_page.body = instance->default_html;

    *handle = instance;
    return ESP_OK;
}

esp_err_t app_web_start(app_web_handle_t handle)
{
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    esp_err_t err;
    size_t page_index;

    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->started) {
        return ESP_OK;
    }

    server_config.server_port = handle->port;
    err = httpd_start(&handle->server, &server_config);
    if (err != ESP_OK) {
        return err;
    }

    if (handle->pages != NULL && handle->page_count > 0) {
        for (page_index = 0; page_index < handle->page_count; ++page_index) {
            err = app_web_register_page(handle->server, &handle->pages[page_index]);
            if (err != ESP_OK) {
                httpd_stop(handle->server);
                handle->server = NULL;
                return err;
            }
        }
    } else {
        err = app_web_register_page(handle->server, &handle->default_page);
        if (err != ESP_OK) {
            httpd_stop(handle->server);
            handle->server = NULL;
            return err;
        }
    }

    handle->started = 1;
    app_logging_infof(handle->logger, "web server started on port %u", handle->port);
    return ESP_OK;
}

esp_err_t app_web_stop(app_web_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!handle->started) {
        return ESP_OK;
    }

    if (handle->server != NULL) {
        httpd_stop(handle->server);
        handle->server = NULL;
    }

    handle->started = 0;
    return ESP_OK;
}

esp_err_t app_web_deinit(app_web_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    app_web_stop(*handle);
    free(*handle);
    *handle = NULL;
    return ESP_OK;
}
