#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
CC="${CC:-cc}"
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT

"$CC" -std=c11 -Wall -Wextra -Werror -pedantic \
    "$ROOT_DIR/tests/framebuffer_range_test.c" \
    -o "$BUILD_DIR/framebuffer_range_test"

"$BUILD_DIR/framebuffer_range_test"
