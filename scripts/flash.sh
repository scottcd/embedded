#!/usr/bin/env bash

set -euo pipefail

IDF_CMD=()

set_env_from_activation_script() {
    local script_path="$1"
    local line
    local key
    local value

    [[ -f "${script_path}" ]] || return 1

    while IFS= read -r line; do
        case "${line}" in
            *=*)
                key="${line%%=*}"
                value="${line#*=}"
                export "${key}=${value}"
                ;;
        esac
    done < <("${script_path}" -e 2>/dev/null)

    return 0
}

find_activation_script() {
    local candidate

    if [[ -f "${HOME}/.espressif/tools/activate_idf_v6.0.sh" ]]; then
        printf '%s\n' "${HOME}/.espressif/tools/activate_idf_v6.0.sh"
        return 0
    fi

    for candidate in "${HOME}"/.espressif/tools/activate_idf_*.sh; do
        if [[ -f "${candidate}" ]]; then
            printf '%s\n' "${candidate}"
            return 0
        fi
    done

    return 1
}

resolve_idf_command() {
    local activation_script
    local idf_python

    if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -n "${IDF_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" && -f "${IDF_PATH}/tools/idf.py" ]]; then
        IDF_CMD=("${IDF_PYTHON_ENV_PATH}/bin/python" "${IDF_PATH}/tools/idf.py")
        return 0
    fi

    if activation_script="$(find_activation_script)"; then
        set_env_from_activation_script "${activation_script}"
        if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -n "${IDF_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" && -f "${IDF_PATH}/tools/idf.py" ]]; then
            IDF_CMD=("${IDF_PYTHON_ENV_PATH}/bin/python" "${IDF_PATH}/tools/idf.py")
            return 0
        fi
    fi

    if command -v idf.py >/dev/null 2>&1; then
        IDF_CMD=("$(command -v idf.py)")
        return 0
    fi

    if [[ -n "${IDF_PATH:-}" && -f "${IDF_PATH}/export.sh" ]]; then
        # shellcheck disable=SC1090
        source "${IDF_PATH}/export.sh" >/dev/null 2>&1
        if command -v idf.py >/dev/null 2>&1; then
            IDF_CMD=("$(command -v idf.py)")
            return 0
        fi
    fi

    if [[ -f "${HOME}/esp/esp-idf/export.sh" ]]; then
        # shellcheck disable=SC1090
        source "${HOME}/esp/esp-idf/export.sh" >/dev/null 2>&1
        if command -v idf.py >/dev/null 2>&1; then
            IDF_CMD=("$(command -v idf.py)")
            return 0
        fi
    fi

    return 1
}

usage() {
    cat <<'EOF'
Usage: ./scripts/flash.sh <target> [options]

Flash an ESP-IDF app from apps/<target>.

Defaults:
  port: /dev/ttyACM0

Options:
  -p, --port <device>     Serial device to use
  -b, --baud <baudrate>   Flash baud rate passed to idf.py
  -h, --help              Show this help text

Any additional arguments are forwarded to idf.py before the flash command.

Examples:
  ./scripts/flash.sh reference_app
  ./scripts/flash.sh reference_app --port /dev/ttyACM1
  ./scripts/flash.sh reference_app --baud 921600
  ./scripts/flash.sh reference_app -DIDF_TARGET=esp32s3

ESP-IDF resolution:
  1. Use official EIM activation metadata from \$HOME/.espressif/tools/activate_idf_*.sh
  2. Else use an already-active ESP-IDF environment
  3. Else fall back to legacy export.sh-based installs
EOF
}

if [[ $# -eq 0 ]]; then
    usage
    exit 1
fi

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    usage
    exit 0
fi

target="$1"
shift

port="/dev/ttyACM0"
baud=""
idf_args=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--port)
            if [[ $# -lt 2 ]]; then
                echo "error: missing value for $1" >&2
                exit 1
            fi
            port="$2"
            shift 2
            ;;
        -b|--baud)
            if [[ $# -lt 2 ]]; then
                echo "error: missing value for $1" >&2
                exit 1
            fi
            baud="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            idf_args+=("$1")
            shift
            ;;
    esac
done

app_dir="apps/${target}"

if [[ ! -d "${app_dir}" ]]; then
    echo "error: target app directory not found: ${app_dir}" >&2
    exit 1
fi

if ! resolve_idf_command; then
    echo "error: idf.py not found" >&2
    echo "source your ESP-IDF environment, set IDF_PATH, or install/activate ESP-IDF via eim" >&2
    exit 1
fi

cmd=("${IDF_CMD[@]}" -C "${app_dir}" -p "${port}")

if [[ -n "${baud}" ]]; then
    cmd+=(-b "${baud}")
fi

cmd+=("${idf_args[@]}" flash)

echo "Flashing ${target} via ${port}"
exec "${cmd[@]}"
