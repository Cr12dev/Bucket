#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ensure built
"${SCRIPT_DIR}/build.sh"

BUILD_DIR="${SCRIPT_DIR}/../build"
GAME_EXE="${BUILD_DIR}/game"

if [ -f "${GAME_EXE}" ]; then
  "${GAME_EXE}"
else
  echo "game executable not found at ${GAME_EXE}" >&2
  exit 1
fi
