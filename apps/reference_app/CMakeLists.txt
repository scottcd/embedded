cmake_minimum_required(VERSION 3.16)

set(IDF_TARGET "esp32s3")
set(EXTRA_COMPONENT_DIRS "../../components")

set(APP_ENV_FILE "${CMAKE_CURRENT_LIST_DIR}/.env")
set(APP_ENV_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(APP_ENV_HEADER "${APP_ENV_GENERATED_DIR}/app_env_config.h")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${APP_ENV_FILE}")

if(NOT EXISTS "${APP_ENV_FILE}")
    message(FATAL_ERROR "Missing ${APP_ENV_FILE}. Create it from ${CMAKE_CURRENT_LIST_DIR}/.env.example before building.")
endif()

file(STRINGS "${APP_ENV_FILE}" APP_ENV_LINES)

foreach(APP_ENV_LINE IN LISTS APP_ENV_LINES)
    string(STRIP "${APP_ENV_LINE}" APP_ENV_LINE)

    if(APP_ENV_LINE STREQUAL "" OR APP_ENV_LINE MATCHES "^#")
        continue()
    endif()

    if(APP_ENV_LINE MATCHES "^WIFI_SSID=(.*)$")
        set(APP_ENV_WIFI_SSID "${CMAKE_MATCH_1}")
    elseif(APP_ENV_LINE MATCHES "^WIFI_PASSWORD=(.*)$")
        set(APP_ENV_WIFI_PASSWORD "${CMAKE_MATCH_1}")
    endif()
endforeach()

if(NOT DEFINED APP_ENV_WIFI_SSID OR APP_ENV_WIFI_SSID STREQUAL "")
    message(FATAL_ERROR "WIFI_SSID is required in ${APP_ENV_FILE}")
endif()

if(NOT DEFINED APP_ENV_WIFI_PASSWORD OR APP_ENV_WIFI_PASSWORD STREQUAL "")
    message(FATAL_ERROR "WIFI_PASSWORD is required in ${APP_ENV_FILE}")
endif()

string(REGEX REPLACE "^\"(.*)\"$" "\\1" APP_ENV_WIFI_SSID "${APP_ENV_WIFI_SSID}")
string(REGEX REPLACE "^\"(.*)\"$" "\\1" APP_ENV_WIFI_PASSWORD "${APP_ENV_WIFI_PASSWORD}")

file(MAKE_DIRECTORY "${APP_ENV_GENERATED_DIR}")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/app_env_config.h.in"
    "${APP_ENV_HEADER}"
    @ONLY
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(reference_app)
