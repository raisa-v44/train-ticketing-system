#ifndef TRAIN_H
#define TRAIN_H
#include "segtree.h"

#define PNR_BASE 1000

typedef enum { ST_CONFIRMED, ST_WAITLISTED, ST_CANCELLED } Status;

typedef struct {
    int pnr, from, to, seats;   /* journey covers segments from .. to-1 */
    Status status;
} Booking;

typedef struct WLNode {          /* waitlist = FIFO linked-list queue */
    int pnr;
    struct WLNode *next;
} WLNode;

typedef struct {
    char name[48];
    int cap;                     /* seats on the train */
    int nstations, nseg;
    const char **names;          /* station names (may be NULL) */
    int naive;                   /* 1 = brute-force array, 0 = segment tree */
    SegTree *st;
    int *arr;
    Booking *bk; int nbk, cbk;   /* all bookings, indexed by pnr - PNR_BASE */
    WLNode *wl_head, *wl_tail; int wl_len;
    int verbose;                 /* print promotions */
} Train;

Train *train_create(const char *name, int cap, int nstations, const char **names, int naive);
void   train_free(Train *t);

/* Returns PNR (>0). -1 = invalid input. -2 = no seats and waitlist not allowed. */
int    train_book(Train *t, int from, int to, int seats, int allow_waitlist);
int    train_cancel(Train *t, int pnr);          /* 1 = cancelled, 0 = not found/already */
int    train_free_seats(Train *t, int from, int to);   /* -1 if invalid */
const Booking *train_get(Train *t, int pnr);
const char *train_station(Train *t, int i);
void   train_chart(Train *t);

#endif
