/* The cast receiver joined to this app (core/cast.c is the part other TV apps reuse). */
#ifndef SB_CASTLINK_H
#define SB_CASTLINK_H
void castlink_start(void);         /* at start: the receiver listens while the app runs */
void castlink_tick(float dt);      /* per frame (before the app rests under the player) */
void castlink_draw(void);          /* the pairing code, over everything */
#endif
