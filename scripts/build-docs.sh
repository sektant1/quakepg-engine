#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"

mkdir -p "$BUILD"
cd "$BUILD"

cmake "$ROOT"
make docs

echo "Docs generated"
