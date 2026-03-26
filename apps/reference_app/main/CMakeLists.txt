idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS
        "."
        "../../../boards/esp32_s3_n16_r8/include"
        "${APP_ENV_GENERATED_DIR}"
    REQUIRES app_dht22 app_logging app_storage app_system app_web app_wifi
)
