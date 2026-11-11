/* Splitting a run of text for a viewer-chosen subtitle font that may lack the neutral characters (an Arabic-only TV font has no
   space, full stop or digits): the font keeps all its letters, and only the missing neutrals go to the app's own font.
   Plain logic, no SDL or FreeType, so tests/runsplit builds it on a host. */
#ifndef SB_RUNSPLIT_H
#define SB_RUNSPLIT_H
#include <stdint.h>
#define RS_MAX 64
typedef int (*RsHas)(void *ctx, uint32_t cp);   /* 1: the chosen font draws it (or the character is invisible and joins any piece) */
int rs_neutral(uint32_t c);                     /* a space, digit or punctuation mark: any font may stand in for it */
/* [a,b) in pieces: pa/pb the bounds, pov 1 = the chosen font, 0 = the app's own. Returns the number of pieces, 0 when the chosen
   font lacks a letter of the run (the whole run then stays in the app's font), -1 when there are more than RS_MAX pieces. */
int rs_split(const uint32_t *cps, int a, int b, RsHas has, void *ctx, int *pa, int *pb, int *pov);
#endif
