idf_component_register(
    SRCS "app_storage.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_common nvs_flash
)
