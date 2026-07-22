#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${TMPDIR:-/tmp}/parol6-exp23-native-tests"
mkdir -p "$BUILD_DIR"

"${CXX:-c++}" \
  -std=c++17 \
  -Wall \
  -Wextra \
  -Werror \
  -pedantic \
  -I"$ROOT/tests/stubs" \
  -I"$ROOT/src" \
  -include "$ROOT/tests/stubs/structs_stub.h" \
  "$ROOT/tests/octopus_board_native_test.cpp" \
  "$ROOT/src/octopus_board.cpp" \
  -o "$BUILD_DIR/octopus_board_native_test"

"$BUILD_DIR/octopus_board_native_test"

PYTHONDONTWRITEBYTECODE=1 \
  python3 "$ROOT/tests/run_upstream_home_native_tests.py"
