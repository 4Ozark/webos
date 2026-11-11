/* Auto-sync against the film's own sound: where someone speaks (one flag per 10 ms, from libsbdts' speech()) against
   where the subtitle shows a line. Every shift within +-range s at each usual frame-rate ratio is scored at once (a
   cross-correlation by FFT): at each the correlation (Pearson's r) of "someone speaks" with "a line is on" over the
   window heard. The answer is new time = old * f + d; it is taken only when its peak stands far above the rest of the
   curve and clearly above the best answer that puts the subtitle elsewhere.
   No allocation outside the call; any thread. */
#ifndef SB_SPEECHSYNC_H
#define SB_SPEECHSYNC_H
typedef struct { double s, e; } SpLine;   /* a subtitle line's original times (s) */
typedef struct {
  double d, f;          /* the answer */
  double score;         /* its correlation (1: lines exactly where the speech is) */
  double margin;        /* over the best answer clearly elsewhere */
  double z;             /* its peak over the curve's spread */
  double speech;        /* seconds of speech heard */
  int lines;            /* the subtitle's lines in the window, at the answer */
  int ok;               /* clear enough to apply */
  int promising;        /* not yet, but more of the film would likely settle it */
} SpAnswer;
/* bits: n flags, the first for t0 .. t0 + 0.01 s. 1 when an answer was worked out (a->ok says whether it is clear), 0
   when there is too little to go by (little speech, few lines, out of memory) */
int speech_match(const unsigned char *bits, int n, double t0, const SpLine *x, int nx, double range, SpAnswer *a);
#endif
