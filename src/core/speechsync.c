#include "core/speechsync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* the same frame-rate ratios the other matchers try (a subtitle timed for a 25 fps release on a 23.976 one, ...), the
   plain one first: a tie goes to it */
static const double RATIOS[7] = { 1, 25 / 23.976, 23.976 / 25, 24 / 23.976, 23.976 / 24, 25.0 / 24, 24.0 / 25 };
#define STEP 0.01
#define PEAKS 6

/* ---- an in-place radix-2 FFT (inv: the inverse, unscaled) ---- */
static void fft(double *re, double *im, int n, int inv) {
  for (int i = 1, j = 0; i < n; i++) { int b = n >> 1; for (; j & b; b >>= 1) j ^= b; j ^= b;
    if (i < j) { double t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; } }
  for (int len = 2; len <= n; len <<= 1) {
    double ang = (inv ? 2 : -2) * M_PI / len, wr = cos(ang), wi = sin(ang);
    for (int i = 0; i < n; i += len) { double cr = 1, ci = 0;
      for (int k = 0; k < len / 2; k++) { int a = i + k, b = a + len / 2;
        double xr = re[b] * cr - im[b] * ci, xi = re[b] * ci + im[b] * cr;
        re[b] = re[a] - xr; im[b] = im[a] - xi; re[a] += xr; im[a] += xi;
        double t = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = t; } } }
}

typedef struct { double sc, f, d; int ri; } Peak;
/* two answers put the subtitle in clearly different places within the window heard: a second apart at either end */
static int apart(double f1, double d1, double f2, double d2, double t0, double t1) { return fabs(t0 * (f1 - f2) + d1 - d2) > 1.0 || fabs(t1 * (f1 - f2) + d1 - d2) > 1.0; }

int speech_match(const unsigned char *bits, int n, double t0, const SpLine *x, int nx, double range, SpAnswer *a) {
  memset(a, 0, sizeof *a); a->f = 1;
  int P = 0; for (int i = 0; i < n; i++) P += bits[i] != 0;
  a->speech = P * STEP;
  if (P < 1000 || P > n - 1000 || nx < 10) return 0; /* (under 10 s of speech heard, or nothing but: nothing to go by) */
  int M = (int)(2 * range / STEP), L = n + M, N = 1; while (N < L) N <<= 1;
  double *ar = calloc((size_t)N, sizeof *ar), *ai = calloc((size_t)N, sizeof *ai), *sr = malloc((size_t)N * sizeof *sr), *si = malloc((size_t)N * sizeof *si);
  unsigned char *on = malloc((size_t)L); double *curve = malloc((size_t)(M + 1) * sizeof *curve); int ret = 0;
  if (!ar || !ai || !sr || !si || !on || !curve) goto out;
  for (int i = 0; i < n; i++) ar[i] = bits[i] ? 1 : 0;
  fft(ar, ai, N, 0);
  double w0 = t0 - range, t1 = t0 + n * STEP; /* the subtitle's signal starts `range` before the window, at no shift */
  Peak best = { -1e18, 1, 0, 0 }, top[7 * PEAKS]; int ntop = 0; double sum = 0, sq = 0; long cnt = 0;
  long *cum = malloc((size_t)(L + 1) * sizeof *cum); if (!cum) goto out;
  double Pn = (double)P / n, va = P * (1 - Pn); /* the speech flags' mean and spread (times n) */
  for (int ri = 0; ri < 7; ri++) { double f = RATIOS[ri];
    memset(on, 0, (size_t)L);
    for (int i = 0; i < nx; i++) { double s = x[i].s * f, e = x[i].e * f; if (e <= s) continue;
      long k0 = (long)ceil((s - w0) / STEP), k1 = (long)ceil((e - w0) / STEP); if (k1 <= 0 || k0 >= L) continue;
      if (k0 < 0) k0 = 0;
      if (k1 > L) k1 = L;
      memset(on + k0, 1, (size_t)(k1 - k0)); }
    cum[0] = 0; for (int k = 0; k < L; k++) cum[k + 1] = cum[k] + on[k];
    for (int k = 0; k < N; k++) { sr[k] = k < L ? on[k] : 0; si[k] = 0; }
    fft(sr, si, N, 0);
    for (int k = 0; k < N; k++) { double r = ar[k] * sr[k] + ai[k] * si[k], im = ar[k] * si[k] - ai[k] * sr[k]; sr[k] = r; si[k] = im; } /* conj(A) S */
    fft(sr, si, N, 1);
    /* at each shift the correlation of speech and lines over the window (Pearson's r: a subtitle dense in lines
       somewhere gains nothing from it, nor speech heard all along from music the detector took for it) */
    for (int m = 0; m <= M; m++) { double c = sr[m] / N, p = (double)(cum[m + n] - cum[m]), vo = p * (1 - p / n);
      double v = vo > 1 ? (c - P * p / n) / sqrt(va * vo) : 0; curve[m] = v; sum += v; sq += v * v; cnt++; }
    /* this ratio's best local peaks (each the highest within half a second) */
    int pk[PEAKS], np = 0;
    for (int m = 0; m <= M; m++) { int lo = m - 50 < 0 ? 0 : m - 50, hi = m + 50 > M ? M : m + 50, is = 1;
      for (int q = lo; q <= hi && is; q++) if (curve[q] > curve[m] || (curve[q] == curve[m] && q < m)) is = 0;
      if (!is) continue;
      if (np < PEAKS) pk[np++] = m; else { int w = 0; for (int q = 1; q < PEAKS; q++) if (curve[pk[q]] < curve[pk[w]]) w = q; if (curve[m] > curve[pk[w]]) pk[w] = m; } }
    for (int p = 0; p < np; p++) { Peak k = { curve[pk[p]], f, range - pk[p] * STEP, ri }; top[ntop++] = k;
      if (k.sc > best.sc) best = k; } }
  free(cum);
  /* the same place at a nearly equal ratio (1 against 24 / 23.976 over a few minutes): the more usual one, unless clearly
     better */
  for (int i = 0; i < ntop; i++) if (top[i].ri < best.ri && !apart(top[i].f, top[i].d, best.f, best.d, t0, t1) && top[i].sc >= best.sc - 0.01) best = top[i];
  if (best.sc <= -1e17) goto out;
  double second = -1e18; for (int i = 0; i < ntop; i++) if (apart(top[i].f, top[i].d, best.f, best.d, t0, t1) && top[i].sc > second) second = top[i].sc;
  double mean = sum / cnt, sd = sqrt(fmax(sq / cnt - mean * mean, 1e-9));
  int lines = 0; for (int i = 0; i < nx; i++) { double s = x[i].s * best.f + best.d, e = x[i].e * best.f + best.d; if (e > t0 && s < t1) lines++; }
  a->d = round(best.d * 100) / 100; a->f = best.f; a->score = best.sc; a->margin = best.sc - second; a->z = (best.sc - mean) / sd; a->lines = lines;
  /* clear: well above every answer elsewhere, high, far above the curve, and lines enough. Measured on four test films
     (real speech over music and noise, 5.1 and stereo, AC3 / E-AC3 / AAC, windows all through them): another film's
     subtitle peaked at margin 0.07, r 0.43, z 4.0 at most; the right one, at margin 0.22, r 0.59, z 4.5 at least */
  a->ok = a->margin >= 0.15 && a->score >= 0.5 && a->z >= 4.4 && lines >= 12;
  a->promising = !a->ok && a->margin >= 0.1 && a->score >= 0.45;
  fprintf(stderr, "[sync] speech: %.0f s heard over %.0f s, %d lines; best %+.2f s x%.4f: score %.2f, margin %.2f, peak z %.1f%s\n",
          a->speech, n * STEP, lines, a->d, a->f, a->score, a->margin, a->z, a->ok ? "" : " (unsure)");
  ret = 1;
out:
  free(ar); free(ai); free(sr); free(si); free(on); free(curve); return ret;
}
