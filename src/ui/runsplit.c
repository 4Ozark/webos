#include "ui/runsplit.h"
int rs_neutral(uint32_t c) {
  return c == ' ' || c == 0xA0 || (c >= '0' && c <= '9') || (c >= 0x21 && c <= 0x2F) || (c >= 0x3A && c <= 0x40) || (c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7E)
      || c == 0xAB || c == 0xBB || (c >= 0x2000 && c <= 0x206F) || c == 0x060C || c == 0x061B || c == 0x061F || (c >= 0x0660 && c <= 0x0669) || (c >= 0x06F0 && c <= 0x06F9);
}
int rs_split(const uint32_t *cps, int a, int b, RsHas has, void *ctx, int *pa, int *pb, int *pov) {
  int np = 0;
  for (int i = a; i < b; i++) {
    int ok = has(ctx, cps[i]);
    if (!ok && !rs_neutral(cps[i])) return 0;
    if (np && pov[np - 1] == ok) { pb[np - 1] = i + 1; continue; }
    if (np >= RS_MAX) return -1;
    pa[np] = i; pb[np] = i + 1; pov[np] = ok; np++;
  }
  return np;
}
