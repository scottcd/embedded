#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <env-file> <output-header>" >&2
    exit 1
fi

env_file="$1"
output_header="$2"

if [[ ! -f "${env_file}" ]]; then
    echo "error: env file not found: ${env_file}" >&2
    echo "create it from ${env_file}.example if available" >&2
    exit 1
fi

wifi_ssid=""
wifi_password=""

while IFS= read -r raw_line || [[ -n "${raw_line}" ]]; do
    line="${raw_line#"${raw_line%%[![:space:]]*}"}"
    line="${line%"${line##*[![:space:]]}"}"

    if [[ -z "${line}" || "${line}" == \#* ]]; then
        continue
    fi

    key="${line%%=*}"
    value="${line#*=}"

    key="${key%"${key##*[![:space:]]}"}"
    value="${value#"${value%%[![:space:]]*}"}"
    value="${value%"${value##*[![:space:]]}"}"

    if [[ "${value}" == \"*\" && "${value}" == *\" ]]; then
        value="${value:1:${#value}-2}"
    fi

    case "${key}" in
        WIFI_SSID)
            wifi_ssid="${value}"
            ;;
        WIFI_PASSWORD)
            wifi_password="${value}"
            ;;
    esac
done < "${env_file}"

if [[ -z "${wifi_ssid}" ]]; then
    echo "error: WIFI_SSID is required in ${env_file}" >&2
    exit 1
fi

if [[ -z "${wifi_password}" ]]; then
    echo "error: WIFI_PASSWORD is required in ${env_file}" >&2
    exit 1
fi

escaped_ssid="${wifi_ssid//\\/\\\\}"
escaped_ssid="${escaped_ssid//\"/\\\"}"
escaped_password="${wifi_password//\\/\\\\}"
escaped_password="${escaped_password//\"/\\\"}"

cat > "${output_header}" <<EOF
#ifndef APP_ENV_CONFIG_H
#define APP_ENV_CONFIG_H

#define APP_ENV_WIFI_SSID "${escaped_ssid}"
#define APP_ENV_WIFI_PASSWORD "${escaped_password}"

#endif
EOF
