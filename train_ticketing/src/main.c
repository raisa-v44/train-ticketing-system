/* Train ticketing engine - interactive CLI, scripted demo and benchmark. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "train.h"

static const char *ROUTE[] = {"Bengaluru", "Tumakuru", "Davangere", "Hubballi",
                              "Belagavi", "Kolhapur", "Pune", "Mumbai"};
#define ROUTE_N 8

static Train *new_default(int cap) {
    return train_create("Demo Express", cap, ROUTE_N, ROUTE, 0);
}

static int ci_prefix(const char *p, const char *s) {
    while (*p) { if (tolower((unsigned char)*p++) != tolower((unsigned char)*s++)) return 0; }
    return 1;
}

static int find_station(Train *t, const char *tok) {
    if (isdigit((unsigned char)tok[0])) { int v = atoi(tok); return v < t->nstations ? v : -1; }
    if (!t->names) return -1;
    for (int i = 0; i < t->nstations; i++) if (ci_prefix(tok, t->names[i])) return i;
    return -1;
}

static const char *status_str(Status s) {
    return s == ST_CONFIRMED ? "CONFIRMED" : s == ST_WAITLISTED ? "WAITLISTED" : "CANCELLED";
}

static void print_help(void) {
    puts("Commands:\n"
         "  route                    show stations and train capacity\n"
         "  book <from> <to> <n>     book n seats (stations by number or name, e.g. book 0 3 2)\n"
         "  cancel <pnr>             cancel a booking (waitlist is auto-promoted)\n"
         "  status <pnr>             show a booking\n"
         "  avail <from> <to>        free seats guaranteed on that journey\n"
         "  chart                    occupancy bars for every segment\n"
         "  waitlist                 show waitlisted bookings in queue order\n"
         "  reset [capacity]         fresh train (default capacity 10)\n"
         "  sim <segments> <ops>     benchmark: segment tree vs naive array\n"
         "  demo                     guided demo scenario\n"
         "  help | quit");
}

/* ---- benchmark ---- */
static double sim_run(int segs, int ops, int naive, int *accepted) {
    Train *t = train_create("SIM", 60, segs + 1, NULL, naive);
    srand(7);
    int acc = 0;
    clock_t s = clock();
    for (int i = 0; i < ops; i++) {
        if (t->nbk > 0 && rand() % 10 < 3) {
            train_cancel(t, PNR_BASE + rand() % t->nbk);
        } else {
            int from = rand() % segs;
            int to = from + 1 + rand() % (segs - from);
            int k = 1 + rand() % 4;
            if (train_book(t, from, to, k, 0) > 0) acc++;
        }
    }
    double secs = (double)(clock() - s) / CLOCKS_PER_SEC;
    train_free(t);
    *accepted = acc;
    return secs;
}

static void do_sim(int segs, int ops) {
    if (segs < 2 || ops < 1) { puts("  usage: sim <segments >= 2> <ops >= 1>"); return; }
    int a1, a2;
    printf("  Running %d random book/cancel operations on a %d-segment route...\n", ops, segs);
    double tn = sim_run(segs, ops, 1, &a1);
    double tt = sim_run(segs, ops, 0, &a2);
    printf("  Naive array  : %.4f s  (%d bookings accepted)\n", tn, a1);
    printf("  Segment tree : %.4f s  (%d bookings accepted)\n", tt, a2);
    printf("  Results %s | speedup: %.1fx\n", a1 == a2 ? "identical" : "MISMATCH!", tt > 0 ? tn / tt : 0.0);
}

static int handle(Train **tp, char *line);

static void do_demo(Train **tp) {
    static const char *script[] = {
        "route", "book 0 3 3", "book Davangere Belagavi 2", "book Pune Mumbai 4",
        "chart", "waitlist", "cancel 1000", "chart", "avail Bengaluru Mumbai"
    };
    train_free(*tp);
    *tp = new_default(4);
    (*tp)->verbose = 1;
    puts("=== Guided demo: a train with only 4 seats ===");
    for (size_t i = 0; i < sizeof script / sizeof *script; i++) {
        printf("\n> %s\n", script[i]);
        char buf[128]; strcpy(buf, script[i]);
        handle(tp, buf);
        printf("  [Enter to continue]");
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) break;
    }
    puts("\n=== Demo finished. You can keep typing commands. ===");
}

/* Returns 0 to quit, 1 to continue. */
static int handle(Train **tp, char *line) {
    Train *t = *tp;
    char cmd[32] = "", a[32] = "", b[32] = "";
    int n = 0, p = 0;
    int got = sscanf(line, "%31s", cmd);
    if (got < 1) return 1;

    if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) return 0;
    else if (!strcmp(cmd, "help")) print_help();
    else if (!strcmp(cmd, "route")) {
        printf("  %s | capacity %d seats | stations:\n", t->name, t->cap);
        for (int i = 0; i < t->nstations; i++) printf("    %d  %s\n", i, train_station(t, i));
    }
    else if (!strcmp(cmd, "book")) {
        if (sscanf(line, "%*s %31s %31s %d", a, b, &n) != 3) { puts("  usage: book <from> <to> <seats>"); return 1; }
        int f = find_station(t, a), to = find_station(t, b);
        if (f < 0 || to < 0) { puts("  unknown station (use 'route' to see the list)"); return 1; }
        int r = train_book(t, f, to, n, 1);
        if (r == -1) puts("  invalid request (need from < to, and 1..capacity seats)");
        else {
            const Booking *bk = train_get(t, r);
            printf("  PNR %d: %s -> %s, %d seat(s): %s", r, train_station(t, f), train_station(t, to), n, status_str(bk->status));
            if (bk->status == ST_WAITLISTED) printf(" (queue position %d)", t->wl_len);
            putchar('\n');
        }
    }
    else if (!strcmp(cmd, "cancel")) {
        if (sscanf(line, "%*s %d", &p) != 1) { puts("  usage: cancel <pnr>"); return 1; }
        if (train_cancel(t, p)) printf("  PNR %d cancelled.\n", p);
        else puts("  PNR not found or already cancelled.");
    }
    else if (!strcmp(cmd, "status")) {
        if (sscanf(line, "%*s %d", &p) != 1) { puts("  usage: status <pnr>"); return 1; }
        const Booking *bk = train_get(t, p);
        if (!bk) puts("  PNR not found.");
        else printf("  PNR %d: %s -> %s, %d seat(s): %s\n", bk->pnr, train_station(t, bk->from),
                    train_station(t, bk->to), bk->seats, status_str(bk->status));
    }
    else if (!strcmp(cmd, "avail")) {
        if (sscanf(line, "%*s %31s %31s", a, b) != 2) { puts("  usage: avail <from> <to>"); return 1; }
        int f = find_station(t, a), to = find_station(t, b);
        int fr = (f < 0 || to < 0) ? -1 : train_free_seats(t, f, to);
        if (fr < 0) puts("  invalid journey.");
        else printf("  %d seat(s) free on every segment of %s -> %s\n", fr, train_station(t, f), train_station(t, to));
    }
    else if (!strcmp(cmd, "chart")) train_chart(t);
    else if (!strcmp(cmd, "waitlist")) {
        if (!t->wl_head) puts("  Waitlist is empty.");
        int pos = 1;
        for (WLNode *w = t->wl_head; w; w = w->next, pos++) {
            const Booking *bk = train_get(t, w->pnr);
            printf("  #%d  PNR %d: %s -> %s, %d seat(s)\n", pos, bk->pnr,
                   train_station(t, bk->from), train_station(t, bk->to), bk->seats);
        }
    }
    else if (!strcmp(cmd, "reset")) {
        int cap = 10;
        sscanf(line, "%*s %d", &cap);
        if (cap < 1) cap = 10;
        train_free(*tp); *tp = new_default(cap);
        printf("  New train with %d seats.\n", cap);
    }
    else if (!strcmp(cmd, "sim")) {
        int segs = 0, ops = 0;
        if (sscanf(line, "%*s %d %d", &segs, &ops) != 2) { puts("  usage: sim <segments> <ops>   e.g. sim 100000 20000"); return 1; }
        do_sim(segs, ops);
    }
    else if (!strcmp(cmd, "demo")) do_demo(tp);
    else puts("  unknown command - type 'help'");
    return 1;
}

int main(void) {
    Train *t = new_default(10);
    t->verbose = 1;
    puts("Train Ticketing Engine - lazy segment tree + waitlist queue");
    puts("Type 'help' for commands, 'demo' for a guided walkthrough.");
    char line[128];
    for (;;) {
        printf("\nticket> ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break;
        if (!handle(&t, line)) break;
    }
    train_free(t);
    return 0;
}
