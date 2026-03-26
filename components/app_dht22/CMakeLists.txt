idf_component_register(
    SRCS "app_dht22.c"
    INCLUDE_DIRS "include"
    REQUIRES app_logging esp_common esp_driver_gpio esp_timer
)
