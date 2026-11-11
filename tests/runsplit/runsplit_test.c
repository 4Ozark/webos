#include "ui/runsplit.h"
#include <stdio.h>
#include <string.h>
static int runs, fails;
#define EXPECT(name, got, want) do { runs++; long g_ = (long)(got), w_ = (long)(want); if (g_ != w_) { fails++; printf("FAIL %s: got %ld, want %ld\n", name, g_, w_); } } while (0)
/* an Arabic-only font: Arabic letters only */
static int ar_only(void *ctx, uint32_t c) { (void)ctx; return (c >= 0x0621 && c <= 0x064A) || (c >= 0x200B && c <= 0x200F); }
static int everything(void *ctx, uint32_t c) { (void)ctx; (void)c; return 1; }
int main(void) {
  int pa[RS_MAX], pb[RS_MAX], pov[RS_MAX];
  EXPECT("space is neutral", rs_neutral(' '), 1); EXPECT("full stop", rs_neutral('.'), 1); EXPECT("digit", rs_neutral('7'), 1); EXPECT("Arabic comma", rs_neutral(0x060C), 1);
  EXPECT("an Arabic letter is not", rs_neutral(0x0628), 0); EXPECT("a Latin letter is not", rs_neutral('a'), 0);
  /* "با با." : letters, a space, letters, a full stop */
  uint32_t s1[] = { 0x0628, 0x0627, ' ', 0x0628, 0x0627, '.' };
  int n = rs_split(s1, 0, 6, ar_only, NULL, pa, pb, pov);
  EXPECT("pieces", n, 4); EXPECT("p0 font", pov[0], 1); EXPECT("p0 range", pa[0] * 10 + pb[0], 2); EXPECT("p1 is the space, app font", pov[1], 0); EXPECT("p1 range", pa[1] * 10 + pb[1], 23);
  EXPECT("p2 font", pov[2], 1); EXPECT("p3 is the full stop, app font", pov[3], 0); EXPECT("p3 range", pa[3] * 10 + pb[3], 56);
  /* a font with everything: one piece */
  n = rs_split(s1, 0, 6, everything, NULL, pa, pb, pov); EXPECT("whole run one piece", n, 1); EXPECT("covers all", pa[0] * 10 + pb[0], 6); EXPECT("chosen font", pov[0], 1);
  /* a Latin letter inside an Arabic-only font: the whole run stays in the app's font */
  uint32_t s2[] = { 0x0628, 'x', 0x0627 }; EXPECT("a missing letter refuses", rs_split(s2, 0, 3, ar_only, NULL, pa, pb, pov), 0);
  /* invisible joiners go with whatever is next to them */
  uint32_t s3[] = { 0x0628, 0x200D, 0x0627 }; EXPECT("joiner stays with the font", rs_split(s3, 0, 3, ar_only, NULL, pa, pb, pov), 1);
  /* a sub-range */
  n = rs_split(s1, 3, 6, ar_only, NULL, pa, pb, pov); EXPECT("sub-range pieces", n, 2); EXPECT("starts at a", pa[0], 3); EXPECT("ends at b", pb[1], 6);
  /* empty */
  EXPECT("empty run", rs_split(s1, 2, 2, ar_only, NULL, pa, pb, pov), 0);
  /* too many pieces */
  uint32_t many[200]; for (int i = 0; i < 200; i++) many[i] = i % 2 ? ' ' : 0x0628; EXPECT("overflow", rs_split(many, 0, 200, ar_only, NULL, pa, pb, pov), -1);
  printf("%d checks, %d failed\n", runs, fails); return fails ? 1 : 0;
}
