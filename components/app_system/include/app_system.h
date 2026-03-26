#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_system app_system_t;
typedef app_system_t *app_system_handle_t;

typedef struct {
    const char *device_name;
} app_system_config_t;

int app_system_init(app_system_handle_t *handle, const app_system_config_t *config);
int app_system_start(app_system_handle_t handle);
int app_system_stop(app_system_handle_t handle);
int app_system_deinit(app_system_handle_t *handle);

const char *app_system_device_name(app_system_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
