/*
  Threaded variant. Steps I want to hit are

  - Create structure to hold all cities (likely one that preserves lexical order
(bi-tree, etc),)
- Mutexes will exist for every city entry
- Open file
- Create enough threads to saturate all cores (16?)
- Handoff pointer to location to threaded process
- threaded process will read in the line and ingest the city data
- final print and calculate process will be handled in main thread, after all
the input has been read

I think the threaded process is going to have some sort of queue
*/
#include "./extras/tpool.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define BUF_SIZE 256
#define WORD_SIZE 100
#define MAX_THREADS 24
#define MAX_ENTRIES 100000
#define MEASUREMENTS_FILE "./measurements_1m.txt"

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

/*
  All of the hashmap related structures + functions defined here
  Give us constant access to city's in array that have already been added
*/
typedef struct {
  char key[WORD_SIZE];
  bool in_use;
  // struct node *node;
  // TODO: add mutex
} HashEntry;

// djb2 hash function - found via http://www.cse.yorku.ca/~oz/hash.html
// (https://stackoverflow.com/questions/7666509/hash-function-for-string)
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash % MAX_ENTRIES;
}

long get_map_index(char *name, HashEntry map[]) {
  long hashval = hash(name);
  while (map[hashval].in_use == true) {
    if (strcmp(map[hashval].key, name) == 0) {
      return hashval;
    } else {
      hashval++;
    }
  }
  return -1;
}

void add_to_map(HashEntry map[], long hashval, char *name) {
  while (map[hashval].in_use == true) {
    hashval++;
    if (hashval > MAX_ENTRIES) {
      fprintf(stderr, "Error: Too many hashmap entries\n");
      exit(1);
    }
  }
  strcpy(map[hashval].key, name);
  map[hashval].in_use = true;
}

// TODO: For now, we'll do something simple like acquire a lock on some
// shared structure and increment a counter
typedef struct {
  int num_rows;
  pthread_mutex_t mut;
} Processed;

void process(void *arg);
int main(void) {
  // static HashEntry map[MAX_ENTRIES];
  Processed rows = {0, PTHREAD_MUTEX_INITIALIZER};
  char buf[BUF_SIZE];
  // _SC_NPROCESSORS_ONLN is the number of currently available procs
  size_t num_threads = (size_t)sysconf(_SC_NPROCESSORS_CONF) / 2;
  if (num_threads > MAX_THREADS)
    num_threads = MAX_THREADS;
  printf("Number of threads configured is %lu\n", num_threads);
  // Create thread pool
  tpool_t *pool = tpool_create(num_threads);
  printf("All threads created\n");
  FILE *file = fopen(MEASUREMENTS_FILE, "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    // Enqueue line in work
    void *to_work = &rows;
    tpool_add_work(pool, process, to_work);
  }
  tpool_wait(pool);
  // Print result after it's done
  // Destroy thread pool
  tpool_destroy(pool);
  printf("All threads completed\n");
  printf("Total rows: %d\n", rows.num_rows);
  return 0;
}

void process(void *arg) {
  int status;
  Processed *to_work = (Processed *)arg;
  while ((status = pthread_mutex_trylock(&to_work->mut)) == EBUSY) {
    sleep(1);
  }
  if (status != 0)
    err_abort(status, "Mutex trylock");
  to_work->num_rows++;
  pthread_mutex_unlock(&to_work->mut);
}