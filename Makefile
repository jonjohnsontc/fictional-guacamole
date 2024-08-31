ifndef NTHREADS
NTHREADS=$(shell nproc --all 2>/dev/null || getconf _NPROCESSORS_ONLN)
endif

CFLAGS=-Wall -Wextra -Werror -O2 -DNTHREADS=$(NTHREADS)

ifdef DEBUG
CFLAGS+=-g
endif

all: bin/ bin/main

bin/:
		mkdir -p bin/

bin/main: 7.c
		$(CC) $(CFLAGS) $^ -o bin/main

.PHONY: clean
clean:
		rm -r bin/