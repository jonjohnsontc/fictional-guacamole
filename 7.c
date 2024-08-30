/*
	Want to add vector intrinsicts to my previous solution

	Current idea is to go through the mmapped data and find n number of rows.
	Those n number of rows can be operated on via vector operations for each
	metric category

	The one issue here is that I could potentially run into the same entry twice
	and then I'd be doing two operations on the same piece of data, which I'm
	assuming would land me with some undefined behavior (or just a crash)
*/
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define BUF_SIZE 256
#define WORD_SIZE 100
#define MAX_ENTRIES 100000
#define HASHMAP_CAPACITY 16384
#define HASHMAP_INDEX(h) (h & (HASHMAP_CAPACITY - 1))
#define MEASUREMENTS_FILE "./measurements_100m.txt"
#define err_abort(code, text)                                                  \
  do {                                                                         \
    fprintf(stderr, "%s at \"%s\":%d:%s\n", text, __FILE__, __LINE__,          \
            strerror(code));                                                   \
    abort();                                                                   \
  } while (0)
#define errno_abort(text)                                                      \
  do {                                                                         \
    fprintf(stderr, "%s at \"%s\":%d:%s\n", text, __FILE__, __LINE__,          \
            strerror(errno));                                                  \
    abort();                                                                   \
  } while (0)
#ifndef NTHREADS
#define NTHREADS 12
#endif 

static size_t chunk_count;
static size_t chunk_size;
static atomic_uint chunk_selector;

typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
} Node;

typedef struct group {
  unsigned int size;
  unsigned int map[HASHMAP_CAPACITY];
  Node rows[MAX_ENTRIES];
} Group;

