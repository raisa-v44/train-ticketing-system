CC     = gcc
CFLAGS = -O2 -Wall -Wextra -g
SRC    = src/main.c src/train.c src/segtree.c

all: ticketing test_train test_bench

ticketing: $(SRC) src/train.h src/segtree.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

test_train: tests/test_train.c src/train.c src/segtree.c src/train.h src/segtree.h
	$(CC) $(CFLAGS) -o $@ tests/test_train.c src/train.c src/segtree.c

test_bench: tests/test_bench.c src/segtree.c src/segtree.h
	$(CC) $(CFLAGS) -o $@ tests/test_bench.c src/segtree.c

run: ticketing
	./ticketing

test: test_train test_bench
	./test_train
	./test_bench

clean:
	rm -f ticketing test_train test_bench *.exe
