#include "core/pvstore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int runs, fails;
#define EXPECT(name, got, want) do { runs++; long g_ = (long)(got), w_ = (long)(want); if (g_ != w_) { fails++; printf("FAIL %s: got %ld, want %ld\n", name, g_, w_); } } while (0)
#define EXPECT_S(name, got, want) do { runs++; const char *g_ = (got), *w_ = (want); if (!g_ || strcmp(g_, w_)) { fails++; printf("FAIL %s: got '%s', want '%s'\n", name, g_ ? g_ : "(null)", w_); } } while (0)
static const char *str(cJSON *r, const char *k) { cJSON *i = cJSON_GetObjectItemCaseSensitive(r, k); return cJSON_IsString(i) ? i->valuestring : NULL; }
static double num(cJSON *r, const char *k, double d) { cJSON *i = cJSON_GetObjectItemCaseSensitive(r, k); return cJSON_IsNumber(i) ? i->valuedouble : d; }
int main(void) {
  cJSON *root = cJSON_CreateObject();
  EXPECT("no record yet", pv_rec(root, "tt1:1:2", 0) == NULL, 1);
  cJSON *e = pv_rec(root, "tt1:1:2", 1); EXPECT("record made", e != NULL, 1);
  pv_set_num(e, "d", 1500); EXPECT("a first delay is kept (the bug: it was dropped)", num(e, "d", -1), 1500);
  pv_set_str(e, "s", "emb|eng|0"); EXPECT_S("a first subtitle is kept", str(e, "s"), "emb|eng|0");
  pv_set_num(e, "d", -250); EXPECT("a later delay replaces it", num(e, "d", 0), -250); EXPECT("and there is only one", cJSON_GetArraySize(e), 2);
  pv_set_str(e, "s", "off"); EXPECT_S("a later subtitle replaces it", str(e, "s"), "off");
  pv_set_num(e, "d", 0); EXPECT("a delay put back to 0 is kept as 0", num(e, "d", -1), 0);
  EXPECT("the same record is found again", pv_rec(root, "tt1:1:2", 0) == e, 1);
  /* a round trip through text, as the store does */
  char *txt = cJSON_PrintUnformatted(root); cJSON *back = cJSON_Parse(txt); free(txt);
  cJSON *b = pv_rec(back, "tt1:1:2", 0); EXPECT("record survives a write and read", b != NULL, 1); EXPECT_S("subtitle survives", str(b, "s"), "off"); EXPECT("delay survives", num(b, "d", -1), 0);
  EXPECT("an empty id makes no record", pv_rec(root, "", 1) == NULL, 1); EXPECT("a null root is harmless", pv_rec(NULL, "x", 1) == NULL, 1);
  pv_set_str(NULL, "k", "v"); pv_set_num(NULL, "k", 1);
  /* pruning: oldest go first, the one being written stays */
  cJSON *many = cJSON_CreateObject(); char id[16];
  for (int i = 0; i < 10; i++) { snprintf(id, sizeof id, "v%d", i); cJSON *r = pv_rec(many, id, 1); pv_set_num(r, "t", 100 + i); }
  cJSON *keepMe = pv_rec(many, "v0", 0);   /* the oldest, but it is the one being written */
  pv_prune(many, 4, keepMe);
  EXPECT("down to the limit", cJSON_GetArraySize(many), 4); EXPECT("the kept one stayed", pv_rec(many, "v0", 0) == keepMe, 1);
  EXPECT("the newest stayed", pv_rec(many, "v9", 0) != NULL, 1); EXPECT("a middle old one went", pv_rec(many, "v1", 0) == NULL, 1);
  pv_prune(many, 100, NULL); EXPECT("under the limit: nothing goes", cJSON_GetArraySize(many), 4);
  cJSON_Delete(root); cJSON_Delete(back); cJSON_Delete(many);
  printf("%d checks, %d failed\n", runs, fails); return fails ? 1 : 0;
}
