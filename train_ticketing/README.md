# Train Ticketing Engine (Lazy Segment Tree)

Experiential Learning, Part B: an advanced data structure not covered in the syllabus,
with a demonstrated application.

**Data structure:** Segment tree with lazy propagation (range-add, range-max)
**Application:** Seat availability and booking for a train with many stations
**Language:** C (no external libraries)

## The problem
A ticket from station *a* to *b* uses every track segment between them. A booking is valid only if
**every** segment in that range still has enough free seats, and booking or cancelling changes
**every** segment in the range.

| Operation the problem needs | Naive array | Segment tree |
|---|---|---|
| "Is the busiest segment in a..b still under capacity?" (range max) | O(L) scan | O(log n) |
| "Add k seats to every segment in a..b" (range update) | O(L) loop | O(log n) |
| Cancel (range update with -k) | O(L) | O(log n) |

L = journey length in segments, n = total segments. Lazy propagation lets a range update touch only
O(log n) nodes by storing the pending add on a parent and pushing it down only when needed.

## Build and run
Using make (Linux/macOS/MSYS2):

    make            # builds ticketing, test_train, test_bench
    make run        # starts the interactive CLI
    make test       # unit tests + correctness check + benchmark

Without make (any OS with gcc), or in VS Code: press **Ctrl+Shift+B** to build, **F5** to debug.

    gcc -O2 -Wall -o ticketing src/main.c src/train.c src/segtree.c

On Windows the executable is `ticketing.exe`; if F5 can't find it, change `program` in
`.vscode/launch.json` to `${workspaceFolder}/ticketing.exe`.

## Commands
    route | book <from> <to> <n> | cancel <pnr> | status <pnr> | avail <from> <to>
    chart | waitlist | reset [capacity] | sim <segments> <ops> | demo | help | quit
Stations can be given by number or by name prefix (`book Pune Mumbai 2`).

## Demo script (about 3 minutes)
1. `make run`, then type `demo` and press Enter through each step. It shows: confirmed booking,
   waitlist when a segment is full, an independent booking elsewhere, and automatic waitlist
   promotion on cancellation, with an occupancy chart before and after.
2. Show scale: `sim 200 20000`, then `sim 20000 20000`, then `sim 100000 20000`.
   The speedup grows with route length. At about 200 segments the naive scan is as fast or faster.
3. Show correctness: `make test` (tree and brute force accept identical bookings over 100k random ops).

## Structure
    src/segtree.[ch]   lazy segment tree (the advanced structure)
    src/train.[ch]     booking records, PNR lookup, waitlist queue, seat checks
    src/main.c         CLI, guided demo, benchmark
    tests/             unit tests (waitlist logic) and tree-vs-brute-force benchmark
    .vscode/           build task (Ctrl+Shift+B) and debugger config (F5)

## Syllabus mapping
| Syllabus topic | Where it appears |
|---|---|
| Queues (FIFO, linked-list implementation) | Waitlist in `train.c` |
| Linked lists (insertion, deletion, traversal) | Waitlist node removal and promotion |
| Trees, traversal, recursion | Segment tree build/query/update |
| Static vs dynamic memory | Dynamic booking array (realloc), tree arrays |
| Complexity analysis | Naive vs tree benchmark |

## Limitations (to state in the report)
- Counts seats per segment; it does not assign specific seat numbers.
- The tree is not faster on short routes (about 200 segments); the benefit appears at network scale.
- Waitlist promotion scans the whole queue, which is O(w) in the waitlist length.
