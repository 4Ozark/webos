/* Continue Watching rules as plain functions: no SDL, no cJSON, so they build and are tested on any host
   (tests/cw/run.sh). stremio.c reads a library item into CwFacts and asks here. */
#ifndef SB_CWLOGIC_H
#define SB_CWLOGIC_H

/* an episode just finished: the series is shown for this long as "up next" (its next episode, nothing played yet) */
#define CW_UPNEXT_MAX_AGE_MS (120.0 * 24 * 3600 * 1000)

typedef struct {
  int isOther;            /* type "other": never shown */
  int removed, temp;      /* removed from the Library (a temp item, one only played, still counts) */
  int hasName;
  int isSeries;
  int videoIdIsEpisode;   /* state.video_id is set and is not the title's own id */
  double timeOffsetMs, durationMs;
  double lastWatchedMs;   /* 0 when unknown */
  double nowMs;
  double dismissedMs;     /* "Remove from Continue Watching" was chosen then (0: never): hidden until something newer is watched */
} CwFacts;

/* a series whose next episode waits: the state points at it with nothing played (offset 0, no duration) and the
   title was watched recently enough */
int cw_up_next(const CwFacts *f);
/* whether the title is in the Continue Watching row: part way through (up to 96 %), or up next */
int cw_visible(const CwFacts *f);

/* leaving an episode (or film) this close to its end counts as having watched it: the last minute or so is credits or an ad.
   True within CW_NEAR_END_S of the end, or from the credits' own start when the skip data gives it (outroHas, outroStartS: only
   believed past the half way mark). Short things (under 10 minutes) and the first minute never count. */
#define CW_NEAR_END_S 100.0

/* New episodes: a series whose last watched episode was the last one out (a season finished) is looked at again now and then;
   when the episode after it comes out, the series is put up next, as if that episode had been known when the last one ended.
   Only titles watched within this long are looked at (a season can take well over a year to come). */
#define CW_NEWEP_MAX_AGE_MS (730.0 * 24 * 3600 * 1000)
typedef struct { int season, episode; double releasedMs; int watched; } CwEp;   /* releasedMs 0: unknown */
/* a title worth looking at: a series (in the Library or only played), its state on an episode, watched lately enough, and not
   already in the row */
int cw_newep_candidate(const CwFacts *f);
/* the episode to put up next after eps[cur] (the one the title's state points at), or -1: eps[cur] must be watched, then the
   following episodes in (season, episode) order are walked past those already watched; the first one not watched is the
   answer when it came out after afterMs (the last watch: a series left while its next episode was already out stays
   left) and by nowMs (a date in the future or none: -1). Specials (season 0) are never part of it. */
int cw_next_aired(const CwEp *eps, int n, int cur, double afterMs, double nowMs);
int cw_near_end(double durationS, double timeS, int outroHas, double outroStartS);

#endif
