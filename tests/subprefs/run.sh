#!/bin/bash
# tests/subprefs/run.sh : builds and runs the subtitle Position / Font table test on this machine (no SDL, no TV needed)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; mkdir -p "$ROOT/build"
cc -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -O1 -g ${SB_ASAN:+-fsanitize=address,undefined} -I"$ROOT/src" "$ROOT/tests/subprefs/subprefs_test.c" "$ROOT/src/core/subprefs.c" -lm -o "$ROOT/build/subprefs_test"
"$ROOT/build/subprefs_test"
