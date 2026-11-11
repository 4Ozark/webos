/* tests/dts/speech_run.sh: Auto-sync against the sound, end to end on a real file: libsbdts' speech() over a window of
   the file, then speech_match() against a subtitle (a plain list of "start end" lines in seconds).
   speech_test <lib> <file> <lines.txt> <from> <dur> [audioOrd] [bits.out] */
#include "core/sbdts.h"
#include "core/speechsync.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static FILE *in; static long long readBytes;
static int rd(void *ud, long long pos, unsigned char *b, int n) { (void)ud; if (fseeko(in, pos, SEEK_SET)) return -1; size_t k = fread(b, 1, n, in); readBytes += (long long)k; return (int)k; }
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
int main(int argc, char **argv) {
  if (argc < 6) { fprintf(stderr, "speech_test <lib> <file> <lines.txt> <from> <dur> [audioOrd] [bits.out]\n"); return 2; }
  void *h = dlopen(argv[1], RTLD_NOW); if (!h) { fprintf(stderr, "%s\n", dlerror()); return 1; }
  const sbd_api *A = ((const sbd_api *(*)(void))dlsym(h, "sbd_get"))();
  if (A->api() < 3) { fprintf(stderr, "library without speech()\n"); return 1; }
  in = fopen(argv[2], "rb"); if (!in) return 1; fseeko(in, 0, SEEK_END); long long size = ftello(in);
  double from = atof(argv[4]), dur = atof(argv[5]); int ord = argc > 6 ? atoi(argv[6]) : -1, cap = (int)(dur * 100) + 1;
  unsigned char *bits = malloc(cap); char err[200] = "";
  double t0 = now(); clock_t c0 = clock();
  int n = A->speech(rd, NULL, size, ord, from, dur, 0, bits, cap, NULL, err, sizeof err);
  double wall = now() - t0, cpu = (double)(clock() - c0) / CLOCKS_PER_SEC;
  if (n <= 0) { fprintf(stderr, "speech: %d %s\n", n, err); return 1; }
  int sp = 0; for (int i = 0; i < n; i++) sp += bits[i]; printf("speech: %d windows, %.1f s of speech, read %.1f MB, cpu %.2f s, wall %.2f s\n", n, sp / 100.0, readBytes / 1048576.0, cpu, wall);
  if (argc > 7) { FILE *o = fopen(argv[7], "wb"); fwrite(bits, 1, n, o); fclose(o); }
  FILE *f = fopen(argv[3], "r"); SpLine *x = NULL; int nx = 0, xc = 0; double s, e;
  while (f && fscanf(f, "%lf %lf", &s, &e) == 2) { if (nx == xc) { xc = xc ? xc * 2 : 1024; x = realloc(x, xc * sizeof *x); } x[nx].s = s; x[nx].e = e; nx++; }
  if (f) fclose(f);
  SpAnswer a; c0 = clock(); int r = speech_match(bits, n, from, x, nx, 150, &a);
  printf("match: %s d=%+.2f f=%.4f score %.2f margin %.2f z %.1f lines %d, cpu %.2f s\n", !r ? "too little" : a.ok ? "OK" : "UNSURE", a.d, a.f, a.score, a.margin, a.z, a.lines, (double)(clock() - c0) / CLOCKS_PER_SEC);
  return 0;
}
