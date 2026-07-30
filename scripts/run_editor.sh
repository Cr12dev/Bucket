#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ensure built
"${SCRIPT_DIR}/build.sh"

BUILD_DIR="${SCRIPT_DIR}/../build"
EDITOR_EXE="${BUILD_DIR}/editor"

if [ -f "${EDITOR_EXE}" ]; then
  "${EDITOR_EXE}"
else
  echo "editor executable not found at ${EDITOR_EXE}" >&2
  exit 1
fi
