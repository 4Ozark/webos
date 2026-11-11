#include "core/pvstore.h"
cJSON *pv_rec(cJSON *root, const char *vid, int make) {
  if (!root || !vid || !*vid) return NULL;
  cJSON *e = cJSON_GetObjectItemCaseSensitive(root, vid);
  if (!e && make) e = cJSON_AddObjectToObject(root, vid);
  return e;
}
/* (cJSON_ReplaceItemInObject does nothing when the key is not there yet, so a new key is added after the old one is dropped) */
void pv_set_str(cJSON *rec, const char *key, const char *value) {
  if (!rec || !key) return;
  cJSON_DeleteItemFromObjectCaseSensitive(rec, key); cJSON_AddStringToObject(rec, key, value ? value : "");
}
void pv_set_num(cJSON *rec, const char *key, double value) {
  if (!rec || !key) return;
  cJSON_DeleteItemFromObjectCaseSensitive(rec, key); cJSON_AddNumberToObject(rec, key, value);
}
static double stamp(const cJSON *c) { const cJSON *t = cJSON_GetObjectItemCaseSensitive((cJSON *)c, "t"); return cJSON_IsNumber(t) ? t->valuedouble : 0; }
void pv_prune(cJSON *root, int max, const cJSON *keep) {
  if (!root || max < 1) return;
  while (cJSON_GetArraySize(root) > max) {
    cJSON *old = NULL; double ot = 1e300;
    for (cJSON *c = root->child; c; c = c->next) { double t = stamp(c); if (c != keep && t < ot) { ot = t; old = c; } }
    if (!old || !old->string) break;
    cJSON_DetachItemViaPointer(root, old); cJSON_Delete(old);
  }
}
