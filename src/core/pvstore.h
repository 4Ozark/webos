/* The per-title memory ("what the viewer chose for this video last time") as plain functions on a cJSON object:
   { "<video id>": { "s": subtitle, "a": audio, "d": delay ms, "t": when } }. No SDL, so tests/pv builds it on a host. */
#ifndef SB_PVSTORE_H
#define SB_PVSTORE_H
#include "cJSON.h"
cJSON *pv_rec(cJSON *root, const char *vid, int make);                 /* the title's record (made when asked) */
void   pv_set_str(cJSON *rec, const char *key, const char *value);     /* sets or replaces */
void   pv_set_num(cJSON *rec, const char *key, double value);
void   pv_prune(cJSON *root, int max, const cJSON *keep);              /* the oldest records out (by "t") until at most max remain; keep is never removed */
#endif
