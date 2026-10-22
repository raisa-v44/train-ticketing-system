/* Correctness check (tree vs brute force) + scaling benchmark. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/segtree.h"

typedef struct { int l, r, k, live; } Booking;

/* Runs the same random booking/cancel workload on either structure.
 * Returns number of accepted bookings; the two modes must agree. */
static int run(int segs, int ops, int cap, int naive, double *secs) {
    srand(42);
    SegTree *t = naive ? NULL : st_create(segs);
    int *arr = naive ? calloc(segs, sizeof(int)) : NULL;
    Booking *bk = malloc(sizeof(Booking) * ops);
    int nb = 0, acc = 0;
    clock_t s = clock();
    for (int i = 0; i < ops; i++) {
        if (nb > 0 && rand() % 10 < 3) {                 /* cancellation */
            Booking *b = &bk[rand() % nb];
            if (b->live) {
                if (naive) for (int j = b->l; j <= b->r; j++) arr[j] -= b->k;
                else st_add(t, b->l, b->r, -b->k);
                b->live = 0;
            }
        } else {                                         /* booking */
            int l = rand() % segs, r = l + rand() % (segs - l), k = 1 + rand() % 4;
            int mx;
            if (naive) { mx = 0; for (int j = l; j <= r; j++) if (arr[j] > mx) mx = arr[j]; }
            else mx = st_max(t, l, r);
            if (mx + k <= cap) {
                if (naive) for (int j = l; j <= r; j++) arr[j] += k;
                else st_add(t, l, r, k);
                bk[nb++] = (Booking){l, r, k, 1};
                acc++;
            }
        }
    }
    *secs = (double)(clock() - s) / CLOCKS_PER_SEC;
    if (t) st_free(t);
    free(arr); free(bk);
    return acc;
}

int main(void) {
    double ts, ns;
    int a = run(199, 100000, 50, 0, &ts);
    int b = run(199, 100000, 50, 1, &ns);
    printf("Correctness (199 segments, 100k ops): tree accepted %d, brute force accepted %d -> %s\n\n",
           a, b, a == b ? "PASS" : "FAIL");
    if (a != b) return 1;

    printf("%-10s %-12s %-12s %-8s\n", "Segments", "Naive (s)", "SegTree (s)", "Speedup");
    int sizes[] = {200, 2000, 20000, 100000};
    for (int i = 0; i < 4; i++) {
        int x = run(sizes[i], 20000, 60, 0, &ts);
        int y = run(sizes[i], 20000, 60, 1, &ns);
        if (x != y) { printf("MISMATCH at %d\n", sizes[i]); return 1; }
        printf("%-10d %-12.4f %-12.4f %.1fx\n", sizes[i], ns, ts, ts > 0 ? ns / ts : 0.0);
    }
    return 0;
}
