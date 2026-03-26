idf_component_register(
    SRCS "app_wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES app_logging esp_common esp_event esp_netif esp_wifi freertos
)
