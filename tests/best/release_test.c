/* The pick keeps to the release played last for the title and leans to releases that carry subtitles; WEBMux is a WEB-DL.
   The case behind it: Presumed Innocent S01E08, a Torrentio RD multi-language WEBMux beside a TorBox Italian release
   (Presunto.Innocente...ITA-ENG) that won on paper and played without a fitting subtitle.
   cc -std=c11 -D_GNU_SOURCE -DSB_BEST_PURE -Isrc -Ithird_party/cjson tests/best/release_test.c src/core/best.c third_party/cjson/cJSON.c -lm */
#include "core/best.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char tbuf[4][256]; static int tn;
const char *T(const char *en) { return en; }
const char *T1(const char *en, const char *k, const char *v) {
  char *o = tbuf[tn++ & 3], key[40]; snprintf(key, sizeof key, "{%s}", k); const char *p = strstr(en, key);
  if (!p) snprintf(o, 256, "%s", en); else snprintf(o, 256, "%.*s%s%s", (int)(p - en), en, v, p + strlen(key)); return o; }
const TvCaps *best_caps(void) { return NULL; }
double best_net_mbps(void) { return 0; }

static int fails;
static void check(int ok, const char *what) { printf("%s  %s\n", ok ? "ok  " : "FAIL", what); if (!ok) fails++; }
static void mk(Stream *s, const char *name, const char *url, const char *title, const char *file) {
  memset(s, 0, sizeof *s); snprintf(s->name, sizeof s->name, "%s", name); snprintf(s->addon, sizeof s->addon, "Torrentio");
  snprintf(s->url, sizeof s->url, "%s", url); snprintf(s->title, sizeof s->title, "%s", title); snprintf(s->filename, sizeof s->filename, "%s", file); }

int main(void) {
  static const TvCaps c = { 2160, 1, 1, 1, 0, 0 };
  Stream rd, tb, other;
  mk(&rd, "[RD+] Torrentio\n4k DV | HDR", "https://torrentio.strem.fun/realdebrid/x/abc/1/Presumed.Innocent.S01E08.mkv",
     "Presumed.Innocent.S01.Eng.Fre.Ger.Ita.Por.Spa.2160p.WEBMux.DV.HDR.DDP5.1.Atmos-MeM\nPresumed.Innocent.S01E08.Eng.Fre.Ger.Ita.Por.Spa.2160p.WEBMux.DV.HDR.DDP5.1.Atmos-MeM.mkv\n\xf0\x9f\x91\xa4 3 \xf0\x9f\x92\xbe 10.54 GB \xe2\x9a\x99\xef\xb8\x8f TorrentGalaxy\n\xf0\x9f\x87\xac\xf0\x9f\x87\xa7 / \xf0\x9f\x87\xae\xf0\x9f\x87\xb9 / \xf0\x9f\x87\xb5\xf0\x9f\x87\xb9 / \xf0\x9f\x87\xaa\xf0\x9f\x87\xb8 / \xf0\x9f\x87\xab\xf0\x9f\x87\xb7 / \xf0\x9f\x87\xa9\xf0\x9f\x87\xaa",
     "Presumed.Innocent.S01E08.Eng.Fre.Ger.Ita.Por.Spa.2160p.WEBMux.DV.HDR.DDP5.1.Atmos-MeM.mkv");
  mk(&tb, "[TB+] Torrentio\n4k DV | HDR", "https://torrentio.strem.fun/torbox/x/def/1",
     "Presunto.Innocente.S01E08.Il.Verdetto.ITA-ENG.2160p.ATVP.WEB-DL.DV.HDR.EAC3.Atmos.5.1-iDN_CreW.mkv\n\xf0\x9f\x91\xa4 1 \xf0\x9f\x92\xbe 9.41 GB \xe2\x9a\x99\xef\xb8\x8f ilCorSaRoNeRo\n\xf0\x9f\x87\xac\xf0\x9f\x87\xa7 / \xf0\x9f\x87\xae\xf0\x9f\x87\xb9",
     "Presunto.Innocente.S01E08.Il.Verdetto.ITA-ENG.2160p.ATVP.WEB-DL.DV.HDR.EAC3.Atmos.5.1-iDN_CreW.mkv");
  mk(&other, "[RD+] Torrentio\n4k DV | HDR", "https://torrentio.strem.fun/realdebrid/x/ghi/1/x.mkv",
     "Presumed.Innocent.S01E08.2160p.ATVP.WEB-DL.DDP5.1.Atmos.DV.HDR.H.265-FLUX.mkv\n\xf0\x9f\x92\xbe 12.1 GB", "Presumed.Innocent.S01E08.2160p.ATVP.WEB-DL.DDP5.1.Atmos.DV.HDR.H.265-FLUX.mkv");
  BestRate r;
  best_set_langs("eng", "ara");

  /* no history: the multi-language release over the Italian one, normal and top quality */
  best_set_prev("");
  { const Stream *l[] = { &tb, &rd }; check(best_pick(l, 2, 45, &c, 500, &r) == &rd, "best fit: the multi-language WEBMux over the Italian ITA-ENG release");
    setenv("SB_BEST_MAX", "1", 1); check(best_pick(l, 2, 45, &c, 500, &r) == &rd, "top quality: the same"); unsetenv("SB_BEST_MAX"); }
  { BestRate a; best_rate(&rd, &c, 500, 45, &a); check(strstr(a.sum, "EAC3") != NULL, "DDP5.1 in the WEBMux name reads as EAC3"); printf("      %s\n", a.sum); }

  /* the last episode was FLUX's: FLUX again, over the multi-language one */
  best_set_prev("Presumed.Innocent.S01E07.2160p.ATVP.WEB-DL.DDP5.1.Atmos.DV.HDR.H.265-FLUX.mkv");
  { const Stream *l[] = { &tb, &rd, &other }; check(best_pick(l, 3, 45, &c, 500, &r) == &other, "best fit: the release played last (same group)");
    setenv("SB_BEST_MAX", "1", 1); check(best_pick(l, 3, 45, &c, 500, &r) == &other, "top quality: the release played last"); unsetenv("SB_BEST_MAX"); }

  /* top quality still never trades the picture for the release: a 1080p of the last group loses to a 4K */
  Stream flux1080; mk(&flux1080, "[RD+] Torrentio\n1080p", "https://torrentio.strem.fun/realdebrid/x/jkl/1/y.mkv",
     "Presumed.Innocent.S01E08.1080p.ATVP.WEB-DL.DDP5.1.Atmos.H.264-FLUX.mkv\n\xf0\x9f\x92\xbe 4.1 GB", "Presumed.Innocent.S01E08.1080p.ATVP.WEB-DL.DDP5.1.Atmos.H.264-FLUX.mkv");
  { const Stream *l[] = { &flux1080, &rd }; setenv("SB_BEST_MAX", "1", 1); check(best_pick(l, 2, 45, &c, 500, &r) == &rd, "top quality: 4K over the last group's 1080p"); unsetenv("SB_BEST_MAX"); }

  /* top quality ignores the line: a slow measured line still gets the 4K */
  best_set_prev("");
  { const Stream *l[] = { &flux1080, &rd }; setenv("SB_BEST_MAX", "1", 1); check(best_pick(l, 2, 45, &c, 5, &r) == &rd, "top quality: 4K on a 5 Mbps line"); unsetenv("SB_BEST_MAX"); }

  /* Interstellar: a Hindi-led dual audio release, played last, against an English one */
  Stream hin, en; best_set_langs("eng", "ara");
  mk(&hin, "[RD+] Torrentio\n4k HDR", "https://torrentio.strem.fun/realdebrid/x/m/1/i.mkv",
     "Interstellar.2014.IMAX.2160p.UHD.BluRay.Hindi.English.DDP5.1.HDR.x265-Telly.mkv\n\xf0\x9f\x92\xbe 24.1 GB\n\xf0\x9f\x87\xae\xf0\x9f\x87\xb3 / \xf0\x9f\x87\xac\xf0\x9f\x87\xa7", "Interstellar.2014.IMAX.2160p.UHD.BluRay.Hindi.English.DDP5.1.HDR.x265-Telly.mkv");
  mk(&en, "[RD+] Torrentio\n4k HDR", "https://torrentio.strem.fun/realdebrid/x/n/1/j.mkv",
     "Interstellar.2014.IMAX.2160p.UHD.BluRay.x265.HDR.DDP5.1-SWTYBLZ.mkv\n\xf0\x9f\x92\xbe 22.3 GB", "Interstellar.2014.IMAX.2160p.UHD.BluRay.x265.HDR.DDP5.1-SWTYBLZ.mkv");
  best_set_prev("Interstellar.2014.IMAX.2160p.UHD.BluRay.Hindi.English.DDP5.1.HDR.x265-Telly.mkv");
  { const Stream *l[] = { &hin, &en }; check(best_pick(l, 2, 169, &c, 500, &r) == &en, "best fit: English over a Hindi-led dual audio, even played last");
    setenv("SB_BEST_MAX", "1", 1); check(best_pick(l, 2, 169, &c, 500, &r) == &en, "top quality: the same"); unsetenv("SB_BEST_MAX"); }
  best_set_prev("");

  /* an Italian viewer keeps the Italian release */
  best_set_langs("ita", "ita");
  { BestRate a; best_rate(&tb, &c, 500, 45, &a); check(a.subs >= 0, "an Italian viewer: the ITA release is not marked down"); }
  printf(fails ? "%d failed\n" : "all passed\n", fails);
  return fails != 0;
}
