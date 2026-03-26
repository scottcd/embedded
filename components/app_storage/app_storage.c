#include "app_storage.h"

#include <stdlib.h>

#include "nvs_flash.h"

struct app_storage {
    const char *namespace_name;
    int started;
};

int app_storage_init(app_storage_handle_t *handle, const app_storage_config_t *config)
{
    app_storage_t *instance;

    if (handle == NULL || config == NULL) {
        return -1;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return -1;
    }

    instance->namespace_name = config->namespace_name;
    *handle = instance;
    return 0;
}

int app_storage_start(app_storage_handle_t handle)
{
    if (handle == NULL) {
        return -1;
    }

    handle->started = (nvs_flash_init() == ESP_OK);
    return handle->started ? 0 : -1;
}

int app_storage_stop(app_storage_handle_t handle)
{
    if (handle == NULL) {
        return -1;
    }

    handle->started = 0;
    return 0;
}

int app_storage_deinit(app_storage_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        return -1;
    }

    free(*handle);
    *handle = NULL;
    return 0;
}
