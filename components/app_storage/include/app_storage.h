#ifndef APP_STORAGE_H
#define APP_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_storage app_storage_t;
typedef app_storage_t *app_storage_handle_t;

typedef struct {
    const char *namespace_name;
} app_storage_config_t;

int app_storage_init(app_storage_handle_t *handle, const app_storage_config_t *config);
int app_storage_start(app_storage_handle_t handle);
int app_storage_stop(app_storage_handle_t handle);
int app_storage_deinit(app_storage_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
