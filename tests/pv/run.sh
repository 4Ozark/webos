#!/bin/bash
# tests/pv/run.sh : builds and runs the per-title memory test on this machine (no SDL, no TV needed)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; mkdir -p "$ROOT/build"
cc -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -O1 -g ${SB_ASAN:+-fsanitize=address,undefined} -I"$ROOT/src" -I"$ROOT/third_party/cjson" \
  "$ROOT/tests/pv/pv_test.c" "$ROOT/src/core/pvstore.c" "$ROOT/third_party/cjson/cJSON.c" -lm -o "$ROOT/build/pv_test"
"$ROOT/build/pv_test"
