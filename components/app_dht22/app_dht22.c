#include "app_dht22.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_rom_sys.h"
#include "esp_timer.h"

struct app_dht22 {
    gpio_num_t data_gpio;
    uint32_t minimum_read_interval_ms;
    int started;
    int64_t last_read_time_us;
    app_logging_handle_t logger;
};

static esp_err_t app_dht22_wait_for_level(gpio_num_t gpio, int expected_level, int timeout_us)
{
    int64_t start_time_us = esp_timer_get_time();

    while ((esp_timer_get_time() - start_time_us) < timeout_us) {
        if (gpio_get_level(gpio) == expected_level) {
            return ESP_OK;
        }
    }

    return ESP_ERR_TIMEOUT;
}

static void app_dht22_prepare_for_read(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(gpio, 1);
    esp_rom_delay_us(10);
    gpio_set_level(gpio, 0);
    esp_rom_delay_us(1200);
    gpio_set_level(gpio, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
}

esp_err_t app_dht22_init(app_dht22_handle_t *handle, const app_dht22_config_t *config)
{
    app_dht22_t *instance;
    gpio_config_t io_config = {0};

    if (handle == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return ESP_ERR_NO_MEM;
    }

    instance->data_gpio = config->data_gpio;
    instance->minimum_read_interval_ms =
        config->minimum_read_interval_ms > 0 ? config->minimum_read_interval_ms : 2500;
    instance->logger = config->logger;

    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    io_config.pin_bit_mask = 1ULL << config->data_gpio;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;

    if (gpio_config(&io_config) != ESP_OK) {
        free(instance);
        return ESP_FAIL;
    }

    gpio_set_level(config->data_gpio, 1);

    *handle = instance;
    return ESP_OK;
}

esp_err_t app_dht22_start(app_dht22_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    handle->started = 1;
    app_logging_infof(
        handle->logger,
        "dht22 ready on GPIO %d with minimum interval %u ms",
        handle->data_gpio,
        handle->minimum_read_interval_ms
    );
    return ESP_OK;
}

esp_err_t app_dht22_stop(app_dht22_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    handle->started = 0;
    return ESP_OK;
}

esp_err_t app_dht22_deinit(app_dht22_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    free(*handle);
    *handle = NULL;
    return ESP_OK;
}

esp_err_t app_dht22_read(app_dht22_handle_t handle, app_dht22_reading_t *reading)
{
    uint8_t data[5] = {0};
    uint16_t humidity_raw;
    uint16_t temperature_raw;
    int bit_index;
    int64_t pulse_start_us;
    int64_t pulse_width_us;
    int64_t now_us;

    if (handle == NULL || reading == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!handle->started) {
        return ESP_ERR_INVALID_STATE;
    }

    now_us = esp_timer_get_time();
    if (handle->last_read_time_us != 0 &&
        (now_us - handle->last_read_time_us) < ((int64_t)handle->minimum_read_interval_ms * 1000)) {
        return ESP_ERR_INVALID_STATE;
    }

    app_dht22_prepare_for_read(handle->data_gpio);

    if (app_dht22_wait_for_level(handle->data_gpio, 0, 100) != ESP_OK ||
        app_dht22_wait_for_level(handle->data_gpio, 1, 100) != ESP_OK ||
        app_dht22_wait_for_level(handle->data_gpio, 0, 100) != ESP_OK) {
        return ESP_ERR_TIMEOUT;
    }

    for (bit_index = 0; bit_index < 40; ++bit_index) {
        if (app_dht22_wait_for_level(handle->data_gpio, 1, 100) != ESP_OK) {
            return ESP_ERR_TIMEOUT;
        }

        pulse_start_us = esp_timer_get_time();
        if (app_dht22_wait_for_level(handle->data_gpio, 0, 100) != ESP_OK) {
            return ESP_ERR_TIMEOUT;
        }

        pulse_width_us = esp_timer_get_time() - pulse_start_us;
        data[bit_index / 8] <<= 1;
        if (pulse_width_us > 45) {
            data[bit_index / 8] |= 1;
        }
    }

    if (((uint8_t)(data[0] + data[1] + data[2] + data[3])) != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    humidity_raw = ((uint16_t)data[0] << 8) | data[1];
    temperature_raw = ((uint16_t)(data[2] & 0x7F) << 8) | data[3];

    reading->humidity_percent = humidity_raw / 10.0f;
    reading->temperature_c = temperature_raw / 10.0f;
    if ((data[2] & 0x80) != 0) {
        reading->temperature_c = -reading->temperature_c;
    }

    handle->last_read_time_us = now_us;
    return ESP_OK;
}
