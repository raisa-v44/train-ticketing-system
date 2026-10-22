#ifndef SEGTREE_H
#define SEGTREE_H

/* Lazy segment tree: range-add, range-max over track segments.
 * Segment i = the stretch between station i and station i+1. */
typedef struct {
    int n;      /* number of segments */
    int *mx;    /* mx[node] = max booked seats in node's range */
    int *lz;    /* lz[node] = pending add not yet pushed to children */
} SegTree;

SegTree *st_create(int n);
void     st_free(SegTree *t);
void     st_add(SegTree *t, int l, int r, int v);  /* add v to segments l..r */
int      st_max(SegTree *t, int l, int r);         /* max over segments l..r */

#endif
