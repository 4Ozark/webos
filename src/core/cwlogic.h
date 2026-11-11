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
int cw_near_end(double durationS, double timeS, int outroHas, double outroStartS);

#endif
