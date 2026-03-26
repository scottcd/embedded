#include "app_dht22.h"
#include "app_logging.h"
#include "app_storage.h"
#include "app_system.h"
#include "app_web.h"
#include "app_wifi.h"
#include "app_env_config.h"
#include "board_profile.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

typedef struct {
    app_dht22_handle_t sensor;
    app_logging_handle_t logger;
    portMUX_TYPE lock;
    float latest_temperature_f;
    float latest_humidity_percent;
    int has_reading;
} reference_app_dht22_task_context_t;

static esp_err_t reference_app_temperature_page_handler(httpd_req_t *request, void *user_ctx)
{
    reference_app_dht22_task_context_t *context = user_ctx;
    char page[1024];
    float temperature_f = 0.0f;
    float humidity_percent = 0.0f;
    int has_reading = 0;
    int page_len;

    portENTER_CRITICAL(&context->lock);
    temperature_f = context->latest_temperature_f;
    humidity_percent = context->latest_humidity_percent;
    has_reading = context->has_reading;
    portEXIT_CRITICAL(&context->lock);

    if (has_reading) {
        page_len = snprintf(
            page,
            sizeof(page),
            "<!doctype html>"
            "<html><head><meta charset=\"utf-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
            "<title>Temperature</title>"
            "<style>"
            "body{font-family:ui-sans-serif,system-ui,sans-serif;background:#f3efe7;color:#1d1d1d;margin:0;padding:2rem;}"
            "main{max-width:42rem;margin:0 auto;background:#fffdf8;border:1px solid #d8c9b4;border-radius:18px;padding:2rem;box-shadow:0 16px 40px rgba(0,0,0,0.08);}"
            ".value{font-size:3rem;margin:0.5rem 0;color:#2a5b4d;}.muted{color:#5b5348;}"
            "a{display:inline-block;margin-top:1.5rem;color:#2a5b4d;text-decoration:none;font-weight:600;}"
            "</style></head><body><main><h1>DHT22 Temperature</h1>"
            "<p class=\"muted\">Latest reading from the reference app sensor task.</p>"
            "<p class=\"value\">%.1f F</p>"
            "<p class=\"muted\">Humidity: %.1f %% RH</p>"
            "<a href=\"/\">Back to reference app</a>"
            "</main></body></html>",
            temperature_f,
            humidity_percent
        );
    } else {
        page_len = snprintf(
            page,
            sizeof(page),
            "<!doctype html>"
            "<html><head><meta charset=\"utf-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
            "<title>Temperature</title>"
            "<style>"
            "body{font-family:ui-sans-serif,system-ui,sans-serif;background:#f3efe7;color:#1d1d1d;margin:0;padding:2rem;}"
            "main{max-width:42rem;margin:0 auto;background:#fffdf8;border:1px solid #d8c9b4;border-radius:18px;padding:2rem;box-shadow:0 16px 40px rgba(0,0,0,0.08);}"
            ".value{font-size:2rem;margin:0.5rem 0;color:#7a6f61;}.muted{color:#5b5348;}"
            "a{display:inline-block;margin-top:1.5rem;color:#2a5b4d;text-decoration:none;font-weight:600;}"
            "</style></head><body><main><h1>DHT22 Temperature</h1>"
            "<p class=\"muted\">Latest reading from the reference app sensor task.</p>"
            "<p class=\"value\">No reading yet</p>"
            "<p class=\"muted\">The sensor is still warming up or is not connected.</p>"
            "<a href=\"/\">Back to reference app</a>"
            "</main></body></html>"
        );
    }

    if (page_len < 0 || (size_t)page_len >= sizeof(page)) {
        return httpd_resp_send_500(request);
    }

    httpd_resp_set_type(request, "text/html");
    return httpd_resp_send(request, page, HTTPD_RESP_USE_STRLEN);
}

static void reference_app_dht22_task(void *task_arg)
{
    reference_app_dht22_task_context_t *context = task_arg;
    app_dht22_reading_t reading = {0};
    float temperature_f;
    esp_err_t err;

    for (;;) {
        err = app_dht22_read(context->sensor, &reading);
        if (err == ESP_OK) {
            temperature_f = (reading.temperature_c * 9.0f / 5.0f) + 32.0f;
            portENTER_CRITICAL(&context->lock);
            context->latest_temperature_f = temperature_f;
            context->latest_humidity_percent = reading.humidity_percent;
            context->has_reading = 1;
            portEXIT_CRITICAL(&context->lock);
            app_logging_infof(
                context->logger,
                "dht22 reading: %.1f F, %.1f %% RH",
                temperature_f,
                reading.humidity_percent
            );
        } else if (err != ESP_ERR_INVALID_STATE) {
            app_logging_warningf(context->logger, "dht22 read failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(BOARD_DHT22_READ_INTERVAL_MS));
    }
}

void app_main(void)
{
    app_dht22_handle_t dht22_handle = NULL;
    app_system_handle_t system_handle = NULL;
    app_logging_handle_t logging_handle = NULL;
    app_storage_handle_t storage_handle = NULL;
    app_web_handle_t web_handle = NULL;
    app_wifi_handle_t wifi_handle = NULL;
    static reference_app_dht22_task_context_t dht22_task_context = {
        .lock = portMUX_INITIALIZER_UNLOCKED,
    };
    esp_err_t err;
    static const app_web_page_t web_pages[] = {
        {
            .uri = "/",
            .content_type = "text/html",
            .body =
                "<!doctype html>"
                "<html><head><meta charset=\"utf-8\">"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<title>Reference App</title>"
                "<style>"
                "body{font-family:ui-sans-serif,system-ui,sans-serif;background:#f5f1e8;color:#1d1d1d;margin:0;padding:2rem;}"
                "main{max-width:42rem;margin:0 auto;background:#fffdf8;border:1px solid #d8c9b4;border-radius:18px;padding:2rem;box-shadow:0 16px 40px rgba(0,0,0,0.08);}"
                "h1{margin-top:0;font-size:2rem;}p{line-height:1.6;color:#4f473d;}strong{color:#2c5b4c;}"
                ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:1rem;margin-top:1.5rem;}"
                ".card{display:block;background:#f3ece0;border:1px solid #dacbb5;border-radius:16px;padding:1.2rem;text-decoration:none;color:#1d1d1d;}"
                ".card:hover{border-color:#2c5b4c;transform:translateY(-1px);}"
                ".card-title{font-weight:700;color:#2c5b4c;margin-bottom:0.4rem;}"
                "</style></head><body><main><h1>Reference App</h1>"
                "<p>The reusable <strong>app_web</strong> module is serving this page.</p>"
                "<p>For a new app, replace this route array with your own pages and dynamic handlers.</p>"
                "<div class=\"grid\">"
                "<a class=\"card\" href=\"/temperature\">"
                "<div class=\"card-title\">Temperature</div>"
                "<div>View the latest DHT22 reading in Fahrenheit.</div>"
                "</a>"
                "</div>"
                "</main></body></html>",
        },
        {
            .uri = "/temperature",
            .content_type = "text/html",
            .handler = reference_app_temperature_page_handler,
            .user_ctx = &dht22_task_context,
        },
    };

    const app_system_config_t system_config = {
        .device_name = BOARD_PROFILE_NAME,
    };
    const app_logging_config_t logging_config = {
        .tag = "reference_app",
        .min_level = APP_LOG_LEVEL_DEBUG,
        .use_color = 1,
    };
    const app_storage_config_t storage_config = {
        .namespace_name = "reference",
    };
    err = app_system_init(&system_handle, &system_config);
    if (err != ESP_OK) {
        return;
    }

    err = app_logging_init(&logging_handle, &logging_config);
    if (err != ESP_OK) {
        return;
    }

    err = app_logging_start(logging_handle);
    if (err != ESP_OK) {
        return;
    }

    err = app_storage_init(&storage_handle, &storage_config);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "storage init failed: %s", esp_err_to_name(err));
        return;
    }

    esp_log_level_set("wifi", ESP_LOG_NONE);

    err = app_wifi_init(&wifi_handle, &(app_wifi_config_t) {
        .ssid = APP_ENV_WIFI_SSID,
        .password = APP_ENV_WIFI_PASSWORD,
        .max_retry_count = 5,
        .max_tx_power_quarter_dbm = 40,
        .logger = logging_handle,
    });
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "wifi init failed: %s", esp_err_to_name(err));
        return;
    }

    err = app_system_start(system_handle);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "system start failed: %s", esp_err_to_name(err));
        return;
    }

    err = app_dht22_init(&dht22_handle, &(app_dht22_config_t) {
        .data_gpio = BOARD_DHT22_GPIO,
        .minimum_read_interval_ms = BOARD_DHT22_READ_INTERVAL_MS,
        .logger = logging_handle,
    });
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "dht22 init failed: %s", esp_err_to_name(err));
        return;
    }

    err = app_web_init(&web_handle, &(app_web_config_t) {
        .logger = logging_handle,
        .app_name = "Reference App",
        .port = 80,
        .pages = web_pages,
        .page_count = sizeof(web_pages) / sizeof(web_pages[0]),
    });
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "web init failed: %s", esp_err_to_name(err));
        return;
    }

    app_logging_infof(logging_handle, "reference app starting on %s", BOARD_PROFILE_NAME);

    err = app_storage_start(storage_handle);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "storage start failed: %s", esp_err_to_name(err));
        return;
    }

    app_logging_infof(logging_handle, "connecting to Wi-Fi SSID '%s'", APP_ENV_WIFI_SSID);
    err = app_wifi_start(wifi_handle);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "wifi start failed: %s", esp_err_to_name(err));
        return;
    }

    err = app_dht22_start(dht22_handle);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "dht22 start failed: %s", esp_err_to_name(err));
        return;
    }

    dht22_task_context.sensor = dht22_handle;
    dht22_task_context.logger = logging_handle;

    err = app_web_start(web_handle);
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "web start failed: %s", esp_err_to_name(err));
        return;
    }

    err = xTaskCreate(
        reference_app_dht22_task,
        "reference_dht22",
        4096,
        &dht22_task_context,
        5,
        NULL
    ) == pdPASS ? ESP_OK : ESP_FAIL;
    if (err != ESP_OK) {
        app_logging_errorf(logging_handle, "dht22 task start failed: %s", esp_err_to_name(err));
        return;
    }

    app_logging_infof(logging_handle, "reference app started");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
