#include "app_wifi.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define APP_WIFI_CONNECTED_BIT BIT0
#define APP_WIFI_FAIL_BIT BIT1

static void app_wifi_copy_credential(uint8_t *dest, size_t dest_size, const char *src)
{
    size_t copy_len;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    copy_len = 0;
    if (src != NULL) {
        copy_len = strnlen(src, dest_size - 1);
        memcpy(dest, src, copy_len);
    }

    dest[copy_len] = '\0';
}

struct app_wifi {
    char ssid[33];
    char password[65];
    int max_retry_count;
    int max_tx_power_quarter_dbm;
    int retry_count;
    int started;
    int connected;
    app_logging_handle_t logger;
    EventGroupHandle_t event_group;
    esp_netif_t *station_netif;
    esp_event_handler_instance_t wifi_event_handler;
    esp_event_handler_instance_t ip_event_handler;
};

static const char *app_wifi_disconnect_reason_name(wifi_err_reason_t reason)
{
    switch (reason) {
        case WIFI_REASON_AUTH_EXPIRE:
            return "AUTH_EXPIRE";
        case WIFI_REASON_AUTH_LEAVE:
            return "AUTH_LEAVE";
        case WIFI_REASON_ASSOC_TOOMANY:
            return "ASSOC_TOOMANY";
        case WIFI_REASON_ASSOC_LEAVE:
            return "ASSOC_LEAVE";
        case WIFI_REASON_ASSOC_NOT_AUTHED:
            return "ASSOC_NOT_AUTHED";
        case WIFI_REASON_DISASSOC_PWRCAP_BAD:
            return "DISASSOC_PWRCAP_BAD";
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
            return "DISASSOC_SUPCHAN_BAD";
        case WIFI_REASON_IE_INVALID:
            return "IE_INVALID";
        case WIFI_REASON_MIC_FAILURE:
            return "MIC_FAILURE";
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            return "HANDSHAKE_TIMEOUT";
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
            return "GROUP_KEY_TIMEOUT";
        case WIFI_REASON_IE_IN_4WAY_DIFFERS:
            return "IE_IN_4WAY_DIFFERS";
        case WIFI_REASON_GROUP_CIPHER_INVALID:
            return "GROUP_CIPHER_INVALID";
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
            return "PAIRWISE_CIPHER_INVALID";
        case WIFI_REASON_AKMP_INVALID:
            return "AKMP_INVALID";
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
            return "UNSUPPORTED_RSN_IE_VERSION";
        case WIFI_REASON_INVALID_RSN_IE_CAP:
            return "INVALID_RSN_IE_CAP";
        case WIFI_REASON_802_1X_AUTH_FAILED:
            return "8021X_AUTH_FAILED";
        case WIFI_REASON_CIPHER_SUITE_REJECTED:
            return "CIPHER_SUITE_REJECTED";
        case WIFI_REASON_BEACON_TIMEOUT:
            return "BEACON_TIMEOUT";
        case WIFI_REASON_NO_AP_FOUND:
            return "NO_AP_FOUND";
        case WIFI_REASON_AUTH_FAIL:
            return "AUTH_FAIL";
        case WIFI_REASON_ASSOC_FAIL:
            return "ASSOC_FAIL";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "HANDSHAKE_TIMEOUT";
        case WIFI_REASON_CONNECTION_FAIL:
            return "CONNECTION_FAIL";
        case WIFI_REASON_AP_TSF_RESET:
            return "AP_TSF_RESET";
        case WIFI_REASON_ROAMING:
            return "ROAMING";
        default:
            return "UNKNOWN";
    }
}

static void app_wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    app_wifi_handle_t handle = arg;
    wifi_event_sta_disconnected_t *disconnect_event = event_data;

    if (handle == NULL) {
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        app_logging_infof(handle->logger, "starting station mode for SSID '%s'", handle->ssid);
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        handle->connected = 0;

        if (handle->retry_count < handle->max_retry_count) {
            handle->retry_count++;
            app_logging_warningf(
                handle->logger,
                "wifi disconnected: reason=%s (%d), retry %d/%d",
                app_wifi_disconnect_reason_name(disconnect_event->reason),
                disconnect_event->reason,
                handle->retry_count,
                handle->max_retry_count
            );
            esp_wifi_connect();
        } else if (handle->event_group != NULL) {
            app_logging_errorf(
                handle->logger,
                "wifi connection failed after %d retries: reason=%s (%d)",
                handle->max_retry_count,
                app_wifi_disconnect_reason_name(disconnect_event->reason),
                disconnect_event->reason
            );
            xEventGroupSetBits(handle->event_group, APP_WIFI_FAIL_BIT);
        }

        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *got_ip_event = event_data;

        handle->retry_count = 0;
        handle->connected = 1;
        app_logging_infof(
            handle->logger,
            "wifi connected, ip=" IPSTR,
            IP2STR(&got_ip_event->ip_info.ip)
        );
        if (handle->event_group != NULL) {
            xEventGroupSetBits(handle->event_group, APP_WIFI_CONNECTED_BIT);
        }
    }
}

esp_err_t app_wifi_init(app_wifi_handle_t *handle, const app_wifi_config_t *config)
{
    app_wifi_t *instance;

    if (handle == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return ESP_ERR_NO_MEM;
    }

    if (config->ssid == NULL || config->password == NULL) {
        free(instance);
        return ESP_ERR_INVALID_ARG;
    }

    snprintf(instance->ssid, sizeof(instance->ssid), "%s", config->ssid);
    snprintf(instance->password, sizeof(instance->password), "%s", config->password);
    instance->max_retry_count = config->max_retry_count > 0 ? config->max_retry_count : 5;
    instance->max_tx_power_quarter_dbm =
        config->max_tx_power_quarter_dbm > 0 ? config->max_tx_power_quarter_dbm : 40;
    instance->logger = config->logger;
    *handle = instance;
    return ESP_OK;
}

esp_err_t app_wifi_start(app_wifi_handle_t handle)
{
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {0};
    esp_err_t err;

    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->started) {
        return ESP_OK;
    }

    handle->event_group = xEventGroupCreate();
    if (handle->event_group == NULL) {
        return ESP_ERR_NO_MEM;
    }

    err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    handle->station_netif = esp_netif_create_default_wifi_sta();
    if (handle->station_netif == NULL) {
        return ESP_FAIL;
    }

    err = esp_wifi_init(&wifi_init_config);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &app_wifi_event_handler,
        handle,
        &handle->wifi_event_handler
    );
    if (err != ESP_OK) {
        return err;
    }

    err = esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &app_wifi_event_handler,
        handle,
        &handle->ip_event_handler
    );
    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        return err;
    }

    app_wifi_copy_credential(wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), handle->ssid);
    app_wifi_copy_credential(wifi_config.sta.password, sizeof(wifi_config.sta.password), handle->password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        return err;
    }

    // Some boards show marginal RF behavior unless TX power is constrained.
    // We apply the configured quarter-dBm limit after esp_wifi_start() because
    // earlier calls can fail with ESP_ERR_WIFI_NOT_STARTED on this ESP-IDF.
    err = esp_wifi_set_max_tx_power(handle->max_tx_power_quarter_dbm);
    if (err != ESP_OK) {
        return err;
    }

    app_logging_infof(
        handle->logger,
        "wifi tx power set to %.2f dBm (%d quarter-dBm units)",
        handle->max_tx_power_quarter_dbm / 4.0f,
        handle->max_tx_power_quarter_dbm
    );

    handle->started = 1;
    return ESP_OK;
}

esp_err_t app_wifi_stop(app_wifi_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!handle->started) {
        return ESP_OK;
    }

    esp_wifi_stop();
    handle->connected = 0;
    handle->started = 0;
    return ESP_OK;
}

esp_err_t app_wifi_deinit(app_wifi_handle_t *handle)
{
    app_wifi_t *instance;

    if (handle == NULL || *handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    instance = *handle;

    if (instance->started) {
        app_wifi_stop(instance);
    }

    if (instance->wifi_event_handler != NULL) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance->wifi_event_handler);
    }

    if (instance->ip_event_handler != NULL) {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance->ip_event_handler);
    }

    if (instance->station_netif != NULL) {
        esp_netif_destroy(instance->station_netif);
    }

    if (instance->event_group != NULL) {
        vEventGroupDelete(instance->event_group);
    }

    free(instance);
    *handle = NULL;
    return ESP_OK;
}

int app_wifi_is_connected(app_wifi_handle_t handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->connected;
}

const char *app_wifi_ssid(app_wifi_handle_t handle)
{
    if (handle == NULL) {
        return NULL;
    }

    return handle->ssid;
}
