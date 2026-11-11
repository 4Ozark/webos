/* The bundled subtitle fonts through the app's own FreeType and HarfBuzz (tests/fonts/run.sh): each file opens, takes the weight axis the
   way core text does, covers the scripts core/subprefs.c says it does, shapes Latin and Arabic without a missing glyph, and rasterises. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H
#include "hb.h"
#include "hb-ft.h"
#include "core/subprefs.h"

static int fails, runs;
#define CHECK(ok, ...) do { runs++; if (!(ok)) { fails++; printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); } } while (0)

static unsigned char *slurp(const char *p, long *n) { FILE *f = fopen(p, "rb"); if (!f) return NULL; fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET); unsigned char *b = malloc(*n); if (fread(b, 1, *n, f) != (size_t)*n) { free(b); b = NULL; } fclose(f); return b; }
/* returns the share of glyphs that are real (not .notdef) for a UTF-8 text shaped in script/direction */
static int shape(hb_font_t *hf, const char *utf8, hb_script_t sc, int rtl, unsigned *nglyphs) {
  hb_buffer_t *b = hb_buffer_create(); hb_buffer_add_utf8(b, utf8, -1, 0, -1);
  hb_buffer_set_direction(b, rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR); hb_buffer_set_script(b, sc); hb_buffer_set_language(b, hb_language_from_string(rtl ? "ar" : "en", -1));
  hb_shape(hf, b, NULL, 0); unsigned n = hb_buffer_get_length(b); hb_glyph_info_t *gi = hb_buffer_get_glyph_infos(b, NULL); int bad = 0;
  for (unsigned i = 0; i < n; i++) if (!gi[i].codepoint) bad++;
  *nglyphs = n; hb_buffer_destroy(b); return bad;
}
int main(int argc, char **argv) {
  const char *dir = argc > 1 ? argv[1] : "assets/fonts"; FT_Library ft; if (FT_Init_FreeType(&ft)) { printf("FreeType would not start\n"); return 2; }
  const char *latin = "I came back to tell you something. 0123456789 ?!", *arabic = "عدتُ لأخبرك بشيء. هذا اختبار للترجمة ١٢٣";
  for (int i = 1; i < SUBFONT_N; i++) {
    const SubFont *sf = subfont_at(i); if (sf->file[0] == '/') { printf("%-9s the TV's own font: not bundled, not checked here\n", sf->id); continue; } char path[512]; snprintf(path, sizeof path, "%s/%s", dir, sf->file); long n = 0; unsigned char *d = slurp(path, &n);
    CHECK(d != NULL, "%s: file missing", path); if (!d) continue;
    FT_Face face; int err = FT_New_Memory_Face(ft, d, n, 0, &face); CHECK(!err, "%s: FreeType refused it (%d)", sf->id, err); if (err) continue;
    CHECK(!strcmp(face->family_name, sf->family) || strstr(face->family_name, sf->family), "%s: family is '%s', the table says '%s'", sf->id, face->family_name, sf->family);
    FT_MM_Var *mm = NULL; int hasW = 0;
    if (!FT_Get_MM_Var(face, &mm) && mm) { FT_Fixed co[8]; for (unsigned a = 0; a < mm->num_axis && a < 8; a++) { co[a] = mm->axis[a].def; if (mm->axis[a].tag == FT_MAKE_TAG('w', 'g', 'h', 't')) { hasW = 1; FT_Fixed w = 600 << 16; if (w < mm->axis[a].minimum) w = mm->axis[a].minimum; if (w > mm->axis[a].maximum) w = mm->axis[a].maximum; co[a] = w; } }
      CHECK(!FT_Set_Var_Design_Coordinates(face, mm->num_axis < 8 ? mm->num_axis : 8, co), "%s: weight axis would not set", sf->id); FT_Done_MM_Var(ft, mm); }
    printf("%-9s %-17s %6ld bytes  variable weight axis: %s\n", sf->id, face->family_name, n, hasW ? "yes" : "no (static file)");
    FT_Set_Pixel_Sizes(face, 0, 48); hb_font_t *hf = hb_ft_font_create_referenced(face); hb_ft_font_set_load_flags(hf, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
    unsigned ng; int bad;
    int covLatin = 1, covAr = 1; for (const char *c = "Iabcxyz0129?!"; *c; c++) if (!FT_Get_Char_Index(face, (unsigned char)*c)) covLatin = 0; for (unsigned cp = 0x0621; cp <= 0x064A; cp++) { if (cp == 0x0640 || (cp >= 0x063B && cp <= 0x0640)) continue; if (!FT_Get_Char_Index(face, cp)) covAr = 0; } /* the assigned letters (U+063B-U+063F are not characters) */
    CHECK(covLatin == sf->latin, "%s: Latin coverage is %d, the table says %d", sf->id, covLatin, sf->latin); CHECK(covAr == sf->arabic, "%s: Arabic coverage is %d, the table says %d", sf->id, covAr, sf->arabic);
    if (sf->latin) { bad = shape(hf, latin, HB_SCRIPT_LATIN, 0, &ng); CHECK(bad == 0 && ng > 20, "%s: Latin shaping left %d missing glyphs of %u", sf->id, bad, ng); }
    if (sf->arabic) { bad = shape(hf, arabic, HB_SCRIPT_ARABIC, 1, &ng); CHECK(bad == 0 && ng > 15, "%s: Arabic shaping left %d missing glyphs of %u", sf->id, bad, ng);
      { /* joining works: the middle ba of three is not the glyph the lone ba is */
        hb_buffer_t *b1 = hb_buffer_create(), *b3 = hb_buffer_create(); hb_buffer_add_utf8(b1, "\xd8\xa8", -1, 0, -1); hb_buffer_add_utf8(b3, "\xd8\xa8\xd8\xa8\xd8\xa8", -1, 0, -1);
        hb_buffer_t *bs[2] = { b1, b3 }; for (int q = 0; q < 2; q++) { hb_buffer_set_direction(bs[q], HB_DIRECTION_RTL); hb_buffer_set_script(bs[q], HB_SCRIPT_ARABIC); hb_buffer_set_language(bs[q], hb_language_from_string("ar", -1)); hb_shape(hf, bs[q], NULL, 0); }
        unsigned n1 = hb_buffer_get_length(b1), n3 = hb_buffer_get_length(b3); hb_glyph_info_t *g1 = hb_buffer_get_glyph_infos(b1, NULL), *g3 = hb_buffer_get_glyph_infos(b3, NULL);
        int same = n3 == 3 * n1; for (unsigned k = 0; same && k < n3; k++) if (g3[k].codepoint != g1[k % n1].codepoint) same = 0; /* (a font may split a letter into base + dots: compare whole sequences) */
        CHECK(n1 > 0 && !same, "%s: Arabic letters did not join (%u glyphs for three, %u for one)", sf->id, n3, n1); hb_buffer_destroy(b1); hb_buffer_destroy(b3); }
    }
    unsigned gid = FT_Get_Char_Index(face, sf->latin ? 'g' : 0x0639); CHECK(!FT_Load_Glyph(face, gid, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING | FT_LOAD_RENDER) && face->glyph->bitmap.width > 0, "%s: a glyph would not rasterise", sf->id);
    hb_font_destroy(hf); FT_Done_Face(face); free(d);
  }
  printf("%d checks, %d failed\n", runs, fails); return fails ? 1 : 0;
}
