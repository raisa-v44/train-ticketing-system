#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "train.h"

/* ---- storage back-end: segment tree or naive array ---- */
static void seg_add(Train *t, int l, int r, int v) {
    if (t->naive) { for (int i = l; i <= r; i++) t->arr[i] += v; }
    else st_add(t->st, l, r, v);
}
static int seg_max(Train *t, int l, int r) {
    if (!t->naive) return st_max(t->st, l, r);
    int m = t->arr[l];
    for (int i = l + 1; i <= r; i++) if (t->arr[i] > m) m = t->arr[i];
    return m;
}

const char *train_station(Train *t, int i) {
    static char bufs[4][24]; static int k = 0;
    if (t->names) return t->names[i];
    char *b = bufs[k++ & 3];
    snprintf(b, 24, "S%d", i);
    return b;
}

Train *train_create(const char *name, int cap, int nst, const char **names, int naive) {
    Train *t = calloc(1, sizeof *t);
    strncpy(t->name, name, sizeof t->name - 1);
    t->cap = cap; t->nstations = nst; t->nseg = nst - 1;
    t->names = names; t->naive = naive;
    if (naive) t->arr = calloc(t->nseg, sizeof(int));
    else       t->st  = st_create(t->nseg);
    t->cbk = 64;
    t->bk = malloc(sizeof(Booking) * t->cbk);
    return t;
}

void train_free(Train *t) {
    if (!t) return;
    if (t->st) st_free(t->st);
    free(t->arr); free(t->bk);
    for (WLNode *n = t->wl_head, *nx; n; n = nx) { nx = n->next; free(n); }
    free(t);
}

/* ---- waitlist queue ---- */
static void wl_enqueue(Train *t, int pnr) {
    WLNode *n = malloc(sizeof *n);
    n->pnr = pnr; n->next = NULL;
    if (t->wl_tail) t->wl_tail->next = n; else t->wl_head = n;
    t->wl_tail = n; t->wl_len++;
}

static void wl_remove(Train *t, int pnr) {
    WLNode *prev = NULL;
    for (WLNode *cur = t->wl_head; cur; prev = cur, cur = cur->next) {
        if (cur->pnr != pnr) continue;
        if (prev) prev->next = cur->next; else t->wl_head = cur->next;
        if (cur == t->wl_tail) t->wl_tail = prev;
        free(cur); t->wl_len--;
        return;
    }
}

/* After seats free up, confirm waitlisted bookings (in arrival order) that now fit. */
static void promote(Train *t) {
    WLNode *prev = NULL, *cur = t->wl_head;
    while (cur) {
        WLNode *next = cur->next;
        Booking *b = &t->bk[cur->pnr - PNR_BASE];
        if (seg_max(t, b->from, b->to - 1) + b->seats <= t->cap) {
            seg_add(t, b->from, b->to - 1, b->seats);
            b->status = ST_CONFIRMED;
            if (t->verbose)
                printf("  >> Waitlist promoted: PNR %d (%s -> %s, %d seat(s)) is now CONFIRMED\n",
                       b->pnr, train_station(t, b->from), train_station(t, b->to), b->seats);
            if (prev) prev->next = next; else t->wl_head = next;
            if (cur == t->wl_tail) t->wl_tail = prev;
            free(cur); t->wl_len--;
        } else prev = cur;
        cur = next;
    }
}

/* ---- public API ---- */
int train_book(Train *t, int from, int to, int seats, int allow_wl) {
    if (from < 0 || to >= t->nstations || from >= to || seats < 1 || seats > t->cap) return -1;
    int fits = seg_max(t, from, to - 1) + seats <= t->cap;
    if (!fits && !allow_wl) return -2;
    if (t->nbk == t->cbk) { t->cbk *= 2; t->bk = realloc(t->bk, sizeof(Booking) * t->cbk); }
    Booking *b = &t->bk[t->nbk];
    b->pnr = PNR_BASE + t->nbk; b->from = from; b->to = to; b->seats = seats;
    t->nbk++;
    if (fits) { seg_add(t, from, to - 1, seats); b->status = ST_CONFIRMED; }
    else      { b->status = ST_WAITLISTED; wl_enqueue(t, b->pnr); }
    return b->pnr;
}

int train_cancel(Train *t, int pnr) {
    int i = pnr - PNR_BASE;
    if (i < 0 || i >= t->nbk) return 0;
    Booking *b = &t->bk[i];
    if (b->status == ST_CANCELLED) return 0;
    Status was = b->status;
    if (was == ST_CONFIRMED) seg_add(t, b->from, b->to - 1, -b->seats);
    else wl_remove(t, pnr);
    b->status = ST_CANCELLED;
    if (was == ST_CONFIRMED) promote(t);
    return 1;
}

int train_free_seats(Train *t, int from, int to) {
    if (from < 0 || to >= t->nstations || from >= to) return -1;
    return t->cap - seg_max(t, from, to - 1);
}

const Booking *train_get(Train *t, int pnr) {
    int i = pnr - PNR_BASE;
    return (i < 0 || i >= t->nbk) ? NULL : &t->bk[i];
}

void train_chart(Train *t) {
    if (t->nseg > 40) { printf("  (chart only shown for routes with <= 40 segments)\n"); return; }
    int W = t->cap < 30 ? t->cap : 30;
    printf("  Seat occupancy per segment (capacity %d)\n", t->cap);
    for (int i = 0; i < t->nseg; i++) {
        int used = seg_max(t, i, i);
        int fill = used * W / t->cap;
        printf("  %-10s -> %-10s [", train_station(t, i), train_station(t, i + 1));
        for (int j = 0; j < W; j++) putchar(j < fill ? '#' : '.');
        printf("] %d/%d\n", used, t->cap);
    }
}
