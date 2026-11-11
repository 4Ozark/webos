#include "core/subprefs.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const char *POS_VAL[SUBPOS_N] = { "-1", "0", "1", "2", "3", "4", "5" };
static const char *POS_LB[SUBPOS_N]  = { "Low", "Default", "+1", "+2", "+3", "+4", "+5" }; /* short: the player's tray column is narrow; +n = n steps up */
/* a step is about 5 % of the screen's height; the lowest sits just 28 units under the default (the TV's safe margin stays) */
static const float POS_OFF[SUBPOS_N] = { -28.0f, 0.0f, 54.0f, 108.0f, 162.0f, 216.0f, 270.0f };

int subpos_index(const char *stored) {
  if (!stored || !*stored) return SUBPOS_DEFAULT;
  for (int i = 0; i < SUBPOS_N; i++) if (!strcmp(stored, POS_VAL[i])) return i;
  return SUBPOS_DEFAULT;
}
const char *subpos_value(int idx) { return POS_VAL[idx < 0 || idx >= SUBPOS_N ? SUBPOS_DEFAULT : idx]; }
const char *subpos_label(int idx) { return POS_LB[idx < 0 || idx >= SUBPOS_N ? SUBPOS_DEFAULT : idx]; }
float subpos_offset(int idx) { return POS_OFF[idx < 0 || idx >= SUBPOS_N ? SUBPOS_DEFAULT : idx]; }

/* the bundled fonts, all SIL Open Font License 1.1 (their licence files beside them in assets/fonts); "auto" is Inter with Noto Sans Arabic for Arabic script */
static const SubFont FONTS[SUBFONT_N] = {
  { "auto",     "Default",          NULL,                 NULL,               1, 1 },
  { "noto",     "Noto Sans Arabic", "NotoSansArabic.ttf", "Noto Sans Arabic", 1, 1 },
  { "cairo",    "Cairo",            "Cairo.ttf",          "Cairo",            1, 1 },
  { "tajawal",  "Tajawal",          "Tajawal-Bold.ttf",   "Tajawal",          1, 1 },
  { "roboto",   "Roboto",           "Roboto.ttf",         "Roboto",           1, 0 },
  { "opensans", "Open Sans",        "OpenSans.ttf",       "Open Sans",        1, 0 },
  { "arimo",    "Arimo (Arial)",    "Arimo.ttf",          "Arimo",            1, 0 },   /* Arial itself is not free to bundle; Arimo is drawn to the same measures */
  { "almarai",  "Almarai",          "Almarai-Bold.ttf",   "Almarai",          1, 1 },
  { "amiri",    "Amiri",            "Amiri.ttf",          "Amiri",            1, 1 },
  { "lato",     "Lato",             "Lato-Bold.ttf",      "Lato",             1, 0 },
};
int subfont_index(const char *stored) {
  if (!stored || !*stored) return 0;
  for (int i = 0; i < SUBFONT_N; i++) if (!strcmp(stored, FONTS[i].id)) return i;
  return 0;
}
const SubFont *subfont_at(int idx) { return &FONTS[idx < 0 || idx >= SUBFONT_N ? 0 : idx]; }
int subfont_arabic_lang(const char *lang) {
  if (!lang) return 0;
  return !strncasecmp(lang, "ar", 2) || !strncasecmp(lang, "fa", 2) || !strncasecmp(lang, "per", 3) || !strncasecmp(lang, "ur", 2);
}
const SubFont *subfont_for_track(const char *stored, const char *lang) {
  const SubFont *f = subfont_at(subfont_index(stored)); if (!f->file) return NULL;
  return (subfont_arabic_lang(lang) ? f->arabic : f->latin) ? f : NULL;
}

static const char *DIM_VAL[SUBDIM_N] = { "0", "1", "2", "3" };
static const char *DIM_LB[SUBDIM_N]  = { "Normal", "Dim 1", "Dim 2", "Dim 3" };
static const float DIM_SC[SUBDIM_N]  = { 1.0f, 0.85f, 0.70f, 0.55f };
int subdim_index(const char *stored) {
  if (!stored || !*stored) return SUBDIM_DEFAULT;
  for (int i = 0; i < SUBDIM_N; i++) if (!strcmp(stored, DIM_VAL[i])) return i;
  return SUBDIM_DEFAULT;
}
const char *subdim_value(int idx) { return DIM_VAL[idx < 0 || idx >= SUBDIM_N ? SUBDIM_DEFAULT : idx]; }
const char *subdim_label(int idx) { return DIM_LB[idx < 0 || idx >= SUBDIM_N ? SUBDIM_DEFAULT : idx]; }
float subdim_scale(int idx) { return DIM_SC[idx < 0 || idx >= SUBDIM_N ? SUBDIM_DEFAULT : idx]; }
unsigned subdim_rgb(unsigned rgb, int idx) {
  float s = subdim_scale(idx); if (s >= 1.0f) return rgb;
  unsigned r = (unsigned)(((rgb >> 16) & 255) * s + 0.5f), g = (unsigned)(((rgb >> 8) & 255) * s + 0.5f), b = (unsigned)((rgb & 255) * s + 0.5f);
  return (rgb & 0xff000000u) | (r << 16) | (g << 8) | b;
}
