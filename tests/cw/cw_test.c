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

  printf("%d checks, %d failed\n", runs, fails);
  return fails ? 1 : 0;
}
