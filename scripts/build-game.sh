#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"
BUILD_TYPE="${1:-Debug}"

mkdir -p "$BUILD"
cd "$BUILD"

cmake "$ROOT" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
make -j"$(nproc)" quakepg_game

echo "Game build complete ($BUILD_TYPE)"
