#!/bin/bash
# tests/cw/run.sh : builds and runs the Continue Watching rules test on this machine (no SDL, no TV needed)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; mkdir -p "$ROOT/build"
cc -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -O1 -g ${SB_ASAN:+-fsanitize=address,undefined} -I"$ROOT/src" \
  "$ROOT/tests/cw/cw_test.c" "$ROOT/src/core/cwlogic.c" -o "$ROOT/build/cw_test"
"$ROOT/build/cw_test"
