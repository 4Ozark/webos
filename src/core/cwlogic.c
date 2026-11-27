#include "core/cwlogic.h"

int cw_up_next(const CwFacts *f) {
  if (!f->isSeries || !f->videoIdIsEpisode) return 0;
  if (f->timeOffsetMs > 0 || f->durationMs > 0) return 0;          /* something was played of it: the normal rule */
  if (f->lastWatchedMs <= 0) return 0;                              /* no date: its age is unknown, so it stays out */
  return f->nowMs - f->lastWatchedMs <= CW_UPNEXT_MAX_AGE_MS;
}

int cw_visible(const CwFacts *f) {
  if (f->isOther || (f->removed && !f->temp) || !f->hasName) return 0;
  if (f->dismissedMs > 0 && f->lastWatchedMs <= f->dismissedMs) return 0;   /* removed from the row, and nothing watched since */
  if (cw_up_next(f)) return 1;
  if (f->timeOffsetMs <= 0) return 0;
  if (f->durationMs > 0 && f->timeOffsetMs > f->durationMs * 0.96) return 0;   /* (as it was: past 96 % counts as finished) */
  return 1;
}

int cw_near_end(double d, double t, int outroHas, double outroS) {
  if (d < 600 || t < 60 || t > d + 1) return 0;
  return d - t <= CW_NEAR_END_S || (outroHas && outroS > d * 0.5 && outroS < d && t >= outroS);
}

int cw_newep_candidate(const CwFacts *f) {
  if (!f->isSeries || f->isOther || (f->removed && !f->temp) || !f->hasName) return 0;
  double lw = f->lastWatchedMs > 0 ? f->lastWatchedMs : f->mtimeMs;
  if (lw <= 0 || f->nowMs - lw > CW_NEWEP_MAX_AGE_MS) return 0;
  return !cw_visible(f);
}

static int ep_after(const CwEp *a, int s, int e) { return a->season > s || (a->season == s && a->episode > e); }
static int ep_before(const CwEp *a, const CwEp *b) { return a->season < b->season || (a->season == b->season && a->episode < b->episode); }
int cw_next_aired(const CwEp *eps, int n, int cur, double afterMs, double nowMs, double dismissedMs) {
  if (!eps || n <= 0) return -1;
  if (cur >= 0 && cur < n && eps[cur].season > 0 && !eps[cur].watched) return -1;   /* on an episode not finished: the normal rules */
  int far = -1; for (int i = 0; i < n; i++) if (eps[i].watched && eps[i].season > 0 && (far < 0 || ep_before(&eps[far], &eps[i]))) far = i;
  if (far < 0) return -1;                                    /* nothing watched */
  int s = eps[far].season, e = eps[far].episode;
  for (;;) {
    int nx = -1;
    for (int i = 0; i < n; i++) if (eps[i].season > 0 && ep_after(&eps[i], s, e) && (nx < 0 || ep_before(&eps[i], &eps[nx]))) nx = i;
    if (nx < 0) return -1;                                   /* nothing after it */
    if (!eps[nx].watched) {
      double r = eps[nx].releasedMs; if (r <= 0 || r > nowMs || (dismissedMs > 0 && r <= dismissedMs)) return -1;
      return (eps[nx].season > s || r > afterMs) ? nx : -1; } /* a new season after a finished one, or out since the last watch */
    s = eps[nx].season; e = eps[nx].episode;                  /* (watched elsewhere already: on to the one after) */
  }
}
