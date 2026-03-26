idf_component_register(
    SRCS "app_web.c"
    INCLUDE_DIRS "include"
    REQUIRES app_logging esp_common esp_http_server
)
