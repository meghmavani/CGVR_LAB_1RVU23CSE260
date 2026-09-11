#!/usr/bin/env bash
# Builds and runs a single lab target, e.g. scripts/run-lab.sh Lab01
set -euo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <LabName>" >&2
    exit 1
fi

LAB="$1"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(find "$ROOT_DIR/out/build" -mindepth 1 -maxdepth 1 -type d | head -n1)"

if [ -z "$BUILD_DIR" ]; then
    echo "No configured build directory found under out/build/. Configure with CMake first." >&2
    exit 1
fi

cmake --build "$BUILD_DIR" --target "$LAB"
exec "$BUILD_DIR/$LAB"
