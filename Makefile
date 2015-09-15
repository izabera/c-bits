CFLAGS = -std=c11 -march=native -s -Ofast -Wall -Wextra -pedantic \
				 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700

CC = gcc

ifeq ($(CC), gcc)
	CFLAGS += -flto
endif

.PHONY: clean all

all: dir cat wc rand xor
	bin/wc -c bin/*

dir:
	mkdir -p bin

cat:
	$(CC) cat.c -o bin/cat $(CFLAGS)

wc:
	$(CC) wc.c -o bin/wc $(CFLAGS)

rand:
	$(CC) rand.c -o bin/rand $(CFLAGS)

xor:
	$(CC) xor.c -o bin/xor $(CFLAGS)

clean:
	rm -rf bin
