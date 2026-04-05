#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_TYPE="${1:-Debug}"

"$ROOT/scripts/build-game.sh" "$BUILD_TYPE"

cd "$ROOT"
./quakepg_game
