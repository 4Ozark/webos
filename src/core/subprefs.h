/* The subtitle look settings that are more than a number: where the lines sit (Position) and which font draws them.
   Plain tables and lookups, no SDL: the settings screen and the player read the same ones, and tests/subprefs builds them on a host. */
#ifndef SB_SUBPREFS_H
#define SB_SUBPREFS_H

/* ---- Position: seven places, stored as "subsPos" = "-1" .. "5" ("0", the default, is where the lines always sat) ---- */
#define SUBPOS_N 7
#define SUBPOS_DEFAULT 1                    /* the index of "0" */
int         subpos_index(const char *stored);   /* the stored text to its index; anything unknown is the default's */
const char *subpos_value(int idx);              /* the text to store */
const char *subpos_label(int idx);              /* English; shown through T() */
float       subpos_offset(int idx);             /* layout units (the screen is 1080 high): below 0 lowers, above raises */

/* ---- Font: the subtitle font, stored as "subsFont" = a font's id ("auto" = the app's own, as it always was) ---- */
typedef struct {
  const char *id, *label;   /* stored value; the name shown (a font's name is not translated) */
  const char *file;         /* under assets/fonts; NULL for "auto" */
  const char *family;       /* the family name inside the file (libass finds its default font by it) */
  int latin, arabic;        /* the scripts the file covers: Latin letters and digits / Arabic script */
} SubFont;
#define SUBFONT_N 10
int            subfont_index(const char *stored);   /* the stored text to its index; anything unknown is "auto" (0) */
const SubFont *subfont_at(int idx);
/* The font file for a styled (ASS) track in language lang ("" when unknown), or NULL to keep the app's own: a track in an
   Arabic-script language takes the choice only when it covers Arabic, any other takes it only when it covers Latin. */
const SubFont *subfont_for_track(const char *stored, const char *lang);
int            subfont_arabic_lang(const char *lang);

/* ---- Brightness: how strongly the subtitle colour shows, stored as "subsDim" = "0" .. "3" (0, the default, is the colour as it is) ---- */
#define SUBDIM_N 4
#define SUBDIM_DEFAULT 0
int         subdim_index(const char *stored);
const char *subdim_value(int idx);
const char *subdim_label(int idx);   /* English; shown through T() */
float       subdim_scale(int idx);   /* what the colour's red, green and blue are multiplied by */
unsigned    subdim_rgb(unsigned rgb, int idx);   /* 0xRRGGBB, dimmed */

#endif
