/* Subtitle Position and Font tables (src/core/subprefs.c) on a host: tests/subprefs/run.sh */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "core/subprefs.h"
static int fails, runs;
#define EXPECT_I(name, got, want) do { runs++; long g_ = (got), w_ = (want); if (g_ != w_) { fails++; printf("FAIL %s: got %ld, want %ld\n", name, g_, w_); } } while (0)
#define EXPECT_S(name, got, want) do { runs++; const char *g_ = (got), *w_ = (want); if (!g_ || strcmp(g_, w_)) { fails++; printf("FAIL %s: got %s, want %s\n", name, g_ ? g_ : "(null)", w_); } } while (0)
int main(void) {
  /* Position: a stored value finds its place; unknown or empty is the default (where the lines always sat) */
  EXPECT_I("default is index 1", SUBPOS_DEFAULT, 1);
  EXPECT_I("\"0\" is the default", subpos_index("0"), SUBPOS_DEFAULT);
  EXPECT_I("no value is the default", subpos_index(NULL), SUBPOS_DEFAULT); EXPECT_I("empty is the default", subpos_index(""), SUBPOS_DEFAULT);
  EXPECT_I("junk is the default", subpos_index("high"), SUBPOS_DEFAULT); EXPECT_I("out of range is the default", subpos_index("9"), SUBPOS_DEFAULT);
  EXPECT_I("\"-1\" is the lowest", subpos_index("-1"), 0); EXPECT_I("\"5\" is the highest", subpos_index("5"), SUBPOS_N - 1);
  EXPECT_I("default does not move", (long)subpos_offset(SUBPOS_DEFAULT), 0);
  int mono = 1; for (int i = 1; i < SUBPOS_N; i++) if (!(subpos_offset(i) > subpos_offset(i - 1))) mono = 0; EXPECT_I("each place is higher than the one before", mono, 1);
  EXPECT_I("lowest is below the default", subpos_offset(0) < 0, 1);
  EXPECT_I("highest stays on the screen (270 of 1080)", subpos_offset(SUBPOS_N - 1) <= 270, 1);
  for (int i = 0; i < SUBPOS_N; i++) EXPECT_I("value round trip", subpos_index(subpos_value(i)), i);
  EXPECT_I("bad index is the default's offset", (long)subpos_offset(-3), 0); EXPECT_I("bad index (high) is the default's offset", (long)subpos_offset(99), 0);
  EXPECT_S("default label", subpos_label(SUBPOS_DEFAULT), "Default");
  /* Font: "auto" is the app's own, as before; unknown values fall back to it */
  EXPECT_I("auto is index 0", subfont_index("auto"), 0); EXPECT_I("no value is auto", subfont_index(NULL), 0); EXPECT_I("junk is auto", subfont_index("comic"), 0);
  EXPECT_I("auto has no file", subfont_at(0)->file == NULL, 1);
  const char *want[] = { "auto", "noto", "cairo", "tajawal", "roboto", "opensans", "arimo", "almarai", "amiri", "lato", "vazirmatn", "plexar", "naskh", "kufi", "lgui", "lguib", "droidn", "droidk" }; for (int i = 0; i < SUBFONT_N; i++) { EXPECT_S("id", subfont_at(i)->id, want[i]); EXPECT_I("id round trip", subfont_index(subfont_at(i)->id), i); }
  EXPECT_I("bad font index is auto", subfont_at(77)->file == NULL, 1);
  for (int i = 1; i < SUBFONT_N; i++) { EXPECT_I("a font has a file", subfont_at(i)->file != NULL && subfont_at(i)->family != NULL, 1); EXPECT_I("a font covers some script", subfont_at(i)->latin || subfont_at(i)->arabic, 1); }
  /* which styled (ASS) tracks take the chosen font: only where it covers the track's script */
  EXPECT_I("auto: never an override", subfont_for_track("auto", "eng") == NULL && subfont_for_track("auto", "ara") == NULL, 1);
  EXPECT_S("Cairo on an English track", subfont_for_track("cairo", "eng")->family, "Cairo"); EXPECT_S("Cairo on an Arabic track", subfont_for_track("cairo", "ara")->family, "Cairo");
  EXPECT_S("Tajawal on a Persian track", subfont_for_track("tajawal", "per")->family, "Tajawal");
  EXPECT_I("Roboto on an Arabic track stays the app's font (it has no Arabic)", subfont_for_track("roboto", "ar") == NULL, 1);
  EXPECT_I("Open Sans on a Farsi track stays the app's font", subfont_for_track("opensans", "fa") == NULL, 1);
  EXPECT_S("Roboto on an English track", subfont_for_track("roboto", "eng")->family, "Roboto");
  EXPECT_S("Noto Sans Arabic on an Arabic track", subfont_for_track("noto", "ar-SA")->family, "Noto Sans Arabic");
  EXPECT_S("unknown language counts as Latin", subfont_for_track("opensans", "")->family, "Open Sans"); EXPECT_S("no language counts as Latin", subfont_for_track("opensans", NULL)->family, "Open Sans");
  EXPECT_I("Urdu is Arabic script", subfont_arabic_lang("urd") || subfont_arabic_lang("ur"), 1); EXPECT_I("English is not", subfont_arabic_lang("eng"), 0);
  /* the TV's own fonts: a path of the TV's, found only when the file is there */
  { char out[600]; EXPECT_I("a bundled font is found under the assets", subfont_path(subfont_at(subfont_index("cairo")), "assets", out, sizeof out), 1); EXPECT_S("its path", out, "assets/fonts/Cairo.ttf");
    EXPECT_I("a TV font the host does not have is skipped", subfont_path(subfont_at(subfont_index("lgui")), "assets", out, sizeof out), 0); EXPECT_S("and leaves no path", out, "");
    EXPECT_I("auto has no file", subfont_path(subfont_at(0), "assets", out, sizeof out), 0);
    EXPECT_S("LG Smart UI family", subfont_at(subfont_index("lgui"))->family, "LG Smart UI AR HE TH"); EXPECT_I("a TV font starts at the root", subfont_at(subfont_index("droidn"))->file[0] == '/', 1); }
  /* brightness */
  EXPECT_I("default dim index", subdim_index(NULL) == SUBDIM_DEFAULT && subdim_index("") == 0 && subdim_index("junk") == 0 && subdim_index("9") == 0, 1);
  for (int i = 0; i < SUBDIM_N; i++) EXPECT_I("dim round trip", subdim_index(subdim_value(i)), i);
  EXPECT_I("normal leaves the colour", subdim_rgb(0xffffffffu, 0) == 0xffffffffu, 1);
  EXPECT_I("each step is dimmer", subdim_scale(1) < subdim_scale(0) && subdim_scale(2) < subdim_scale(1) && subdim_scale(3) < subdim_scale(2), 1);
  EXPECT_I("white dim 3 is a grey", (subdim_rgb(0xffffffffu, 3) & 0xffffff) == 0x8c8c8c, 1);
  EXPECT_I("alpha is kept", (subdim_rgb(0xffffffffu, 2) >> 24) == 0xff, 1);
  EXPECT_I("black stays black", subdim_rgb(0xff000000u, 3) == 0xff000000u, 1);
  EXPECT_I("a bad index is normal", subdim_scale(-1) == 1.0f && subdim_scale(99) == 1.0f, 1);
  EXPECT_S("Arimo family", subfont_at(subfont_index("arimo"))->family, "Arimo"); EXPECT_I("Arimo has no Arabic, so an Arabic track keeps the app's font", subfont_for_track("arimo", "ara") == NULL, 1);
  EXPECT_I("Amiri covers Arabic", subfont_for_track("amiri", "ar") != NULL, 1);
  printf("%d checks, %d failed\n", runs, fails); return fails ? 1 : 0;
}
