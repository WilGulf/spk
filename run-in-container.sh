#!/usr/bin/env bash

set -euo pipefail

IMAGE_TAG="pkgmgr-testenv"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${SCRIPT_DIR}"

podman build -t "${IMAGE_TAG}" "${SCRIPT_DIR}"

podman run --rm -it \
    -v "${SRC_DIR}:/src:Z" \
    -w /src \
    "${IMAGE_TAG}" \
    "${@:-/bin/bash}"