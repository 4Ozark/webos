#!/bin/bash
# tests/fonts/run.sh : builds the project's own FreeType and HarfBuzz (as tools/build.sh does, for this machine) and checks every
# bundled subtitle font through them. The first run compiles the libraries (a few minutes); build/hosttp is reused after.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"; TP="$ROOT/third_party"; B="$ROOT/build/hosttp"; mkdir -p "$B"
CC="${CC:-cc}"; CXX="${CXX:-c++}"
if [ ! -f "$B/libft_hb.a" ]; then
  echo "== FreeType + HarfBuzz for this machine"
  FT="-DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H=<ftmodule-sb.h> -I$TP/config -I$TP/freetype/include"
  for f in base/ftbase base/ftinit base/ftsystem base/ftdebug base/ftbitmap base/ftglyph base/ftmm base/ftbbox base/ftsynth base/ftstroke \
           gzip/ftgzip truetype/truetype sfnt/sfnt smooth/smooth autofit/autofit psnames/psnames pshinter/pshinter cff/cff psaux/psaux; do
    $CC -w -O1 $FT -c "$TP/freetype/src/$f.c" -o "$B/ft_$(basename $f).o" &
  done; wait
  $CXX -w -O1 -std=c++11 -fno-exceptions -fno-rtti -DHAVE_FREETYPE=1 -DHAVE_PTHREAD=1 -I"$TP/freetype/include" -I"$TP/harfbuzz/src" -c "$TP/harfbuzz/src/harfbuzz.cc" -o "$B/harfbuzz.o"
  ar rcs "$B/libft_hb.a" "$B"/*.o
fi
$CC -std=c11 -D_GNU_SOURCE -Wall -Wextra -O1 -g -I"$ROOT/src" -I"$TP/config" -I"$TP/freetype/include" -I"$TP/harfbuzz/src" "$ROOT/tests/fonts/font_test.c" "$ROOT/src/core/subprefs.c" \
  "$B/libft_hb.a" -lstdc++ -lpthread -lm -o "$B/font_test"
"$B/font_test" "$ROOT/assets/fonts"
