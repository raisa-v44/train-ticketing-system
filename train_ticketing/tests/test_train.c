/* Unit tests for the booking + waitlist layer. */
#include <stdio.h>
#include <assert.h>
#include "../src/train.h"

int main(void) {
    for (int naive = 0; naive <= 1; naive++) {
        Train *t = train_create("T", 4, 8, NULL, naive);
        int p1 = train_book(t, 0, 3, 3, 1);            /* segs 0..2 : 3 seats */
        int p2 = train_book(t, 2, 4, 2, 1);            /* seg 2 would hit 5 > 4 -> waitlist */
        int p3 = train_book(t, 6, 7, 4, 1);            /* separate segment -> confirmed */
        assert(train_get(t, p1)->status == ST_CONFIRMED);
        assert(train_get(t, p2)->status == ST_WAITLISTED);
        assert(train_get(t, p3)->status == ST_CONFIRMED);
        assert(t->wl_len == 1);
        assert(train_free_seats(t, 0, 3) == 1);
        assert(train_free_seats(t, 6, 7) == 0);

        assert(train_book(t, 3, 3, 1, 1) == -1);       /* from == to */
        assert(train_book(t, 0, 3, 5, 1) == -1);       /* more seats than capacity */
        assert(train_book(t, 0, 8, 1, 1) == -1);       /* station out of range */
        assert(train_book(t, 0, 3, 3, 0) == -2);       /* full, no waitlist */

        assert(train_cancel(t, p1) == 1);              /* frees seats -> promote p2 */
        assert(train_get(t, p2)->status == ST_CONFIRMED);
        assert(t->wl_len == 0);
        assert(train_cancel(t, p1) == 0);              /* double cancel */
        assert(train_cancel(t, 9999) == 0);

        int p4 = train_book(t, 2, 4, 3, 1);            /* waitlisted, then cancelled directly */
        assert(train_get(t, p4)->status == ST_WAITLISTED);
        assert(train_cancel(t, p4) == 1);
        assert(t->wl_len == 0);
        train_free(t);
    }
    puts("test_train: all assertions passed (tree and naive back-ends)");
    return 0;
}
