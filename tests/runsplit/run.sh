#!/bin/bash
# tests/runsplit/run.sh : builds and runs the run-splitting test (TV fonts that lack spaces and punctuation) on this machine
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; mkdir -p "$ROOT/build"
cc -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -O1 -g ${SB_ASAN:+-fsanitize=address,undefined} -I"$ROOT/src" \
  "$ROOT/tests/runsplit/runsplit_test.c" "$ROOT/src/ui/runsplit.c" -o "$ROOT/build/runsplit_test"
"$ROOT/build/runsplit_test"
