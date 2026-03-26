#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: ./scripts/install_deps.sh [options]

Install repository dependencies for Linux hosts using Espressif's EIM workflow.

Defaults:
  ESP-IDF version: v6.0
  ESP target: esp32s3

Options:
  --idf-version <version>   ESP-IDF version to install with eim
  --target <target>         ESP target passed to eim install
  --skip-idf               Only install system packages and eim-cli
  --skip-apt               Skip apt repository/package setup and only use existing eim
  -h, --help               Show this help text
EOF
}

require_command() {
    local name="$1"

    if ! command -v "${name}" >/dev/null 2>&1; then
        echo "error: required command not found: ${name}" >&2
        exit 1
    fi
}

configure_espressif_apt_repo() {
    local architecture
    local repo_line

    require_command dpkg
    require_command sudo
    require_command apt-get

    architecture="$(dpkg --print-architecture)"
    repo_line="deb [arch=${architecture} trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main"

    echo "Configuring Espressif apt repository for ${architecture}"
    printf '%s\n' "${repo_line}" | sudo tee /etc/apt/sources.list.d/espressif.list >/dev/null
    sudo apt-get update
    sudo apt-get install -y ca-certificates curl git python3 eim-cli
}

ensure_eim_installation() {
    local version="$1"
    local target="$2"

    require_command eim

    if ! eim list 2>/dev/null | grep -F -- "- ${version} " >/dev/null; then
        echo "Installing ESP-IDF ${version} for target ${target}"
        eim install --idf-versions "${version}" --target "${target}" --non-interactive true
    else
        echo "ESP-IDF ${version} is already installed"
    fi

    echo "Selecting ESP-IDF ${version}"
    eim select "${version}"
}

idf_version="v6.0"
target="esp32s3"
skip_idf=0
skip_apt=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --idf-version)
            if [[ $# -lt 2 ]]; then
                echo "error: missing value for $1" >&2
                exit 1
            fi
            idf_version="$2"
            shift 2
            ;;
        --target)
            if [[ $# -lt 2 ]]; then
                echo "error: missing value for $1" >&2
                exit 1
            fi
            target="$2"
            shift 2
            ;;
        --skip-idf)
            skip_idf=1
            shift
            ;;
        --skip-apt)
            skip_apt=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

if [[ "${skip_apt}" -eq 0 ]]; then
    configure_espressif_apt_repo
else
    require_command eim
fi

if [[ "${skip_idf}" -eq 0 ]]; then
    ensure_eim_installation "${idf_version}" "${target}"
fi

activation_script="${HOME}/.espressif/tools/activate_idf_${idf_version}.sh"

echo
echo "Dependency setup complete."
if [[ -f "${activation_script}" ]]; then
    echo "Activate ESP-IDF with:"
    echo "  source \"${activation_script}\""
else
    echo "Activation script not found at the default path:"
    echo "  ${activation_script}"
    echo "Run 'eim list' and inspect ~/.espressif/tools/ for the generated activation script."
fi
