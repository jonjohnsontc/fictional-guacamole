/*

I'm thinking that leveraging mmap as well as threads would allow me to better
shape the problem for threads.

I think to parallelize the problem better, I'd have to read the file in parallel
and operate using `nthreads` number of streams to the file. If I mmap the input
file, I can just copy `nthreads` number of pointers and have them operate on
contiguous regions of it.

- statically allocate space for structure which holds data. Made up of:
  - array for data
  - hash-array for hashmap for O(1) access to data
- Initialize N number of threads
- mmap input file
- divide memory region into N contiguous regions, each delimited/ended with a
newline character
- compute statistics for each region, and return pointer to results in memory
- merge all results in single thread
- compute and print final result in main thread

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
#include <time.h>
#include <unistd.h>
#define BUF_SIZE 256
#define WORD_SIZE 100
#define MAX_THREADS 24
#define MAX_ENTRIES 100000
#define HASHMAP_CAPACITY 16384
#define HASHMAP_INDEX(h) (h & (HASHMAP_CAPACITY - 1))
#define MEASUREMENTS_FILE "./measurements_1m.txt"
#define err_abort(code, text)                                                  \
  do {                                                                         \
    fprintf(stderr, "%s at \"%s\":%d:%s\n", text, __FILE__, __LINE__,          \
            strerror(code));                                                   \
    abort();                                                                   \
  } while (0)

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

int main(int argc, char *argv[]) {
  int fd;
  char *addr;
  size_t size;
  fd = open("./measurements_1b.txt", O_RDONLY);
  if (fd == -1)
    err_abort(fd, "file open");

  size = lseek(fd, 0, SEEK_END);
  if (size == -1)
    err_abort(size, "lseek");

  addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (*addr == -1)
    err_abort(*addr, "mmap");
  printf("mmapped input file\n");

  return 0;
}
