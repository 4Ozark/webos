/* Continue Watching rules (src/core/cwlogic.c) on a host: tests/cw/run.sh */
#include <stdio.h>
#include <string.h>
#include "core/cwlogic.h"

static int fails, runs;
#define EXPECT(name, got, want) do { runs++; int g_ = (got), w_ = (want); if (g_ != w_) { fails++; printf("FAIL %s: got %d, want %d\n", name, g_, w_); } } while (0)

#define DAY (24.0 * 3600 * 1000)
static const double NOW = 1.8e12;
static CwFacts base(void) { CwFacts f; memset(&f, 0, sizeof f); f.hasName = 1; f.nowMs = NOW; return f; }

int main(void) {
  CwFacts f;
  /* what already worked: part way through shows; nothing played, or finished (past 96 %), does not */
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; EXPECT("part way", cw_visible(&f), 1);
  f = base(); f.timeOffsetMs = 1200000 * 0.97; f.durationMs = 1200000; EXPECT("finished (97 %)", cw_visible(&f), 0);
  f = base(); f.timeOffsetMs = 1200000 * 0.96; f.durationMs = 1200000; EXPECT("exactly 96 % still shows", cw_visible(&f), 1);
  f = base(); f.timeOffsetMs = 5000; EXPECT("offset without a duration", cw_visible(&f), 1);
  f = base(); EXPECT("a film never played", cw_visible(&f), 0);
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.isOther = 1; EXPECT("type other", cw_visible(&f), 0);
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.removed = 1; EXPECT("removed from the library", cw_visible(&f), 0);
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.removed = 1; f.temp = 1; EXPECT("temp (only played) shows", cw_visible(&f), 1);
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.hasName = 0; EXPECT("no name", cw_visible(&f), 0);

  /* the fix: a finished episode leaves the series up next (video id = next episode, nothing played) */
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; EXPECT("up next shows", cw_visible(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; EXPECT("up next is recognised", cw_up_next(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 119 * DAY; EXPECT("up next, 119 days old", cw_visible(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 121 * DAY; EXPECT("up next, 121 days old: stale", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = 0; EXPECT("up next without a date: out", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.lastWatchedMs = NOW - 1000; EXPECT("series, video id is the title's own: out", cw_visible(&f), 0);
  f = base(); f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; EXPECT("a film with an episode-like id: out", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; f.isOther = 1; EXPECT("up next, type other: out", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; f.removed = 1; EXPECT("up next, removed: out", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 1000; f.hasName = 0; EXPECT("up next, no name: out", cw_visible(&f), 0);
  /* an episode marked watched by hand keeps its duration: not up next */
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - 1000; EXPECT("watched by hand (duration kept): out", cw_visible(&f), 0);
  /* part way through the next episode: the normal rule, no longer 'up next' */
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.timeOffsetMs = 90000; f.durationMs = 1500000; f.lastWatchedMs = NOW; EXPECT("next episode started", cw_up_next(&f), 0);
  EXPECT("next episode started shows", cw_visible(&f), 1);

  /* Remove from Continue Watching: hidden until something newer is watched */
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.lastWatchedMs = NOW - 5000; f.dismissedMs = NOW - 1000; EXPECT("removed from the row", cw_visible(&f), 0);
  f = base(); f.timeOffsetMs = 60000; f.durationMs = 1200000; f.lastWatchedMs = NOW - 500; f.dismissedMs = NOW - 1000; EXPECT("watched again after removing: back", cw_visible(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - 5000; f.dismissedMs = NOW - 1000; EXPECT("up next removed", cw_visible(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = 0; f.timeOffsetMs = 60000; f.durationMs = 1200000; f.dismissedMs = NOW - 1000; EXPECT("removed, no date", cw_visible(&f), 0);

  /* new episodes: which titles are looked at */
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - 200 * DAY; EXPECT("season finished 200 days ago: looked at", cw_newep_candidate(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - 800 * DAY; EXPECT("over two years: not", cw_newep_candidate(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - DAY; f.removed = 1; EXPECT("removed from the Library: not", cw_newep_candidate(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - DAY; f.removed = 1; f.temp = 1; EXPECT("only played (temp): looked at", cw_newep_candidate(&f), 1);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.lastWatchedMs = NOW - DAY; EXPECT("already up next: not", cw_newep_candidate(&f), 0);
  f = base(); f.isSeries = 1; f.videoIdIsEpisode = 1; f.timeOffsetMs = 60000; f.durationMs = 1200000; f.lastWatchedMs = NOW - DAY; EXPECT("part way through: not", cw_newep_candidate(&f), 0);
  f = base(); f.videoIdIsEpisode = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - DAY; EXPECT("a film: not", cw_newep_candidate(&f), 0);
  f = base(); f.isSeries = 1; f.durationMs = 1200000; f.lastWatchedMs = NOW - DAY; EXPECT("no episode in the state: not", cw_newep_candidate(&f), 0);

  /* new episodes: which episode */
  { CwEp e[] = { { 1, 1, NOW - 400 * DAY, 1 }, { 1, 2, NOW - 393 * DAY, 1 }, { 2, 1, NOW - 2 * DAY, 0 }, { 2, 2, NOW + 5 * DAY, 0 }, { 0, 1, NOW - 100 * DAY, 0 } };
    double LW = NOW - 390 * DAY; /* season 1 finished then */
    EXPECT("season 2 is out: its first episode", cw_next_aired(e, 5, 1, LW, NOW), 2);
    EXPECT("from an episode not watched: none", cw_next_aired(e, 5, 2, LW, NOW), -1);
    EXPECT("not out yet", cw_next_aired(e, 5, 1, LW, NOW - 3 * DAY), -1);
    EXPECT("out before the last watch (a series left): none", cw_next_aired(e, 5, 1, NOW - DAY, NOW), -1);
    EXPECT("from a special: none", cw_next_aired(e, 5, 4, LW, NOW), -1);
    EXPECT("a bad index", cw_next_aired(e, 5, 9, LW, NOW), -1); }
  { CwEp e[] = { { 2, 1, NOW - 2 * DAY, 1 }, { 1, 2, NOW - 393 * DAY, 1 }, { 1, 1, NOW - 400 * DAY, 1 } }; /* (any order in the meta) */
    EXPECT("the last one out, all watched: none", cw_next_aired(e, 3, 1, NOW - 30 * DAY, NOW), -1); }
  { CwEp e[] = { { 1, 2, NOW - 393 * DAY, 1 }, { 2, 2, NOW - DAY, 0 }, { 2, 1, NOW - 8 * DAY, 1 }, { 1, 1, NOW - 400 * DAY, 1 } };
    EXPECT("watched elsewhere already: the one after it", cw_next_aired(e, 4, 0, NOW - 7 * DAY, NOW), 1); }
  { CwEp e[] = { { 1, 1, NOW - 400 * DAY, 1 }, { 2, 1, 0, 0 } };
    EXPECT("no date: none", cw_next_aired(e, 2, 0, NOW - 390 * DAY, NOW), -1); }

  /* leaving near the end counts as watched */
  EXPECT("100 s left of 45 min", cw_near_end(2700, 2600, 0, 0), 1);
  EXPECT("101 s left is not yet", cw_near_end(2700, 2599, 0, 0), 0);
  EXPECT("a minute and a half left", cw_near_end(2700, 2610, 0, 0), 1);
  EXPECT("half way", cw_near_end(2700, 1350, 0, 0), 0);
  EXPECT("credits start known, past it", cw_near_end(2700, 2400, 1, 2380), 1);
  EXPECT("credits start known, before it", cw_near_end(2700, 2300, 1, 2380), 0);
  EXPECT("a credits start in the first half is not believed", cw_near_end(2700, 1000, 1, 900), 0);
  EXPECT("a credits start past the end is not believed", cw_near_end(2700, 2500, 1, 2800), 0);
  EXPECT("a short clip never counts", cw_near_end(300, 290, 0, 0), 0);
  EXPECT("the first minute never counts", cw_near_end(620, 30, 0, 0), 0);
  EXPECT("no duration", cw_near_end(0, 0, 0, 0), 0);
  printf("%d checks, %d failed\n", runs, fails);
  return fails ? 1 : 0;
}
