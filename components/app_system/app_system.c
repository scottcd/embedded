#include "app_system.h"

#include <stdlib.h>

struct app_system {
    const char *device_name;
    int started;
};

int app_system_init(app_system_handle_t *handle, const app_system_config_t *config)
{
    app_system_t *instance;

    if (handle == NULL || config == NULL) {
        return -1;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        return -1;
    }

    instance->device_name = config->device_name;
    *handle = instance;
    return 0;
}

int app_system_start(app_system_handle_t handle)
{
    if (handle == NULL) {
        return -1;
    }

    handle->started = 1;
    return 0;
}

int app_system_stop(app_system_handle_t handle)
{
    if (handle == NULL) {
        return -1;
    }

    handle->started = 0;
    return 0;
}

int app_system_deinit(app_system_handle_t *handle)
{
    if (handle == NULL || *handle == NULL) {
        return -1;
    }

    free(*handle);
    *handle = NULL;
    return 0;
}

const char *app_system_device_name(app_system_handle_t handle)
{
    if (handle == NULL) {
        return NULL;
    }

    return handle->device_name;
}
