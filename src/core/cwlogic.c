#include "core/cwlogic.h"

int cw_up_next(const CwFacts *f) {
  if (!f->isSeries || !f->videoIdIsEpisode) return 0;
  if (f->timeOffsetMs > 0 || f->durationMs > 0) return 0;          /* something was played of it: the normal rule */
  if (f->lastWatchedMs <= 0) return 0;                              /* no date: its age is unknown, so it stays out */
  return f->nowMs - f->lastWatchedMs <= CW_UPNEXT_MAX_AGE_MS;
}

int cw_visible(const CwFacts *f) {
  if (f->isOther || (f->removed && !f->temp) || !f->hasName) return 0;
  if (cw_up_next(f)) return 1;
  if (f->timeOffsetMs <= 0) return 0;
  if (f->durationMs > 0 && f->timeOffsetMs > f->durationMs * 0.96) return 0;   /* (as it was: past 96 % counts as finished) */
  return 1;
}

int cw_near_end(double d, double t, int outroHas, double outroS) {
  if (d < 600 || t < 60 || t > d + 1) return 0;
  return d - t <= CW_NEAR_END_S || (outroHas && outroS > d * 0.5 && outroS < d && t >= outroS);
}
