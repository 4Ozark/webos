#!/bin/bash
# tests/dts/speech_run.sh <file> <lines.txt> <from> [dur]: Auto-sync against the sound on a real file, with
# build/mac/libsbdts.dylib (tools/build-ffmpeg.sh mac). lines.txt: the subtitle's lines, "start end" in seconds, one a
# line. Prints how much speech was heard (and what reading it cost) and the answer: the shift (and speed) that lines
# the subtitle up, with how clear it is. A subtitle shifted by a known amount should come back with that amount.
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; LIB="${LIB:-$ROOT/build/mac/libsbdts.dylib}"
cc -std=c11 -O2 -D_GNU_SOURCE -I"$ROOT/src" "$ROOT/tests/dts/speech_test.c" "$ROOT/src/core/speechsync.c" -lm -o "$ROOT/build/speech_test"
"$ROOT/build/speech_test" "$LIB" "$1" "$2" "$3" "${4:-240}"
