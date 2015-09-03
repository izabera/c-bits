CFLAGS = -Wall -Wextra -pedantic -std=c11 -march=native -s -Ofast
CC = gcc

.PHONY: clean all

all: dir cat wc
	bin/wc -c bin/*

dir:
	mkdir -p bin

cat:
	$(CC) cat.c -o bin/cat $(CFLAGS)

wc:
	$(CC) wc.c -o bin/wc $(CFLAGS)

clean:
	rm -rf bin
