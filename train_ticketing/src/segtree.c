#include <stdlib.h>
#include "segtree.h"

static void apply(SegTree *t, int node, int v) {
    t->mx[node] += v;
    t->lz[node] += v;
}

static void push(SegTree *t, int node) {
    if (t->lz[node]) {
        apply(t, 2 * node, t->lz[node]);
        apply(t, 2 * node + 1, t->lz[node]);
        t->lz[node] = 0;
    }
}

static void add(SegTree *t, int node, int lo, int hi, int l, int r, int v) {
    if (r < lo || hi < l) return;
    if (l <= lo && hi <= r) { apply(t, node, v); return; }
    push(t, node);
    int mid = (lo + hi) / 2;
    add(t, 2 * node, lo, mid, l, r, v);
    add(t, 2 * node + 1, mid + 1, hi, l, r, v);
    int a = t->mx[2 * node], b = t->mx[2 * node + 1];
    t->mx[node] = a > b ? a : b;
}

static int query(SegTree *t, int node, int lo, int hi, int l, int r) {
    if (r < lo || hi < l) return -2147483647;
    if (l <= lo && hi <= r) return t->mx[node];
    push(t, node);
    int mid = (lo + hi) / 2;
    int a = query(t, 2 * node, lo, mid, l, r);
    int b = query(t, 2 * node + 1, mid + 1, hi, l, r);
    return a > b ? a : b;
}

SegTree *st_create(int n) {
    SegTree *t = malloc(sizeof *t);
    t->n = n;
    t->mx = calloc(4 * (size_t)n, sizeof(int));
    t->lz = calloc(4 * (size_t)n, sizeof(int));
    return t;
}

void st_free(SegTree *t) { free(t->mx); free(t->lz); free(t); }
void st_add(SegTree *t, int l, int r, int v) { add(t, 1, 0, t->n - 1, l, r, v); }
int  st_max(SegTree *t, int l, int r)        { return query(t, 1, 0, t->n - 1, l, r); }
