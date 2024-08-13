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

void msleep(long milliseconds) {
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = milliseconds * 1000000L;
  while (nanosleep(&req, &rem) == -1) {
    if (errno == EINTR) {
      req = rem;
    } else {
      perror("nanosleep");
      exit(1);
    }
  }
}

void pthread_trylock_buffer(pthread_mutex_t *mutex) {
  int status;
  while ((status = pthread_mutex_trylock(mutex)) == EBUSY) {
    msleep(1);
  }
  if (status != 0)
    err_abort(status, "Mutex trylock");
}

/* All of the hashmap related structures +
  functions defined here Give us constant access to
  city's in array that have already been added */
typedef struct {
  char key[WORD_SIZE];
  bool in_use;
  int count;
  // struct node *node;
  pthread_mutex_t mutex;
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
  // Let's find space for the entry in the hashmap
  while (map[hashval].in_use == true) {
    hashval++;
    if (hashval > MAX_ENTRIES) {
      fprintf(stderr, "Error: Too many hashmap entries\n");
      exit(1);
    }
  }
  // Let's lock the thread for the entry, so nobody else can touch it
  pthread_trylock_buffer(&map[hashval].mutex);
  strcpy(map[hashval].key, name);
  map[hashval].in_use = true;
  pthread_mutex_unlock(&map[hashval].mutex);
}

// TODO: For now, we'll do something simple like acquire a lock on some
// shared structure and increment a counter
typedef struct {
  int num_rows;
  char buffer[WORD_SIZE];
  HashEntry *map_ref;
  pthread_mutex_t mut;
} Processed;

void process(void *arg);
void print_map(HashEntry *map, int n);
int main(void) {
  static HashEntry map[MAX_ENTRIES] = {
      [0 ... MAX_ENTRIES - 1] = {"", false, 0, PTHREAD_MUTEX_INITIALIZER}};
  Processed rows = {0, "", map, PTHREAD_MUTEX_INITIALIZER};
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
    pthread_trylock_buffer(&rows.mut);
    strcpy(rows.buffer, buf);
    pthread_mutex_unlock(&rows.mut);
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
  print_map(map, MAX_ENTRIES);
  return 0;
}

void process(void *arg) {
  int idx;
  float temp;
  char name[WORD_SIZE];
  Processed *to_work = (Processed *)arg;
  pthread_trylock_buffer(&to_work->mut);
  to_work->num_rows++;
  sscanf(to_work->buffer, "%[^;];%f", name, &temp);

  if ((idx = get_map_index(to_work->buffer, to_work->map_ref)) == -1) {
    unsigned long hashval = hash(to_work->buffer);
    add_to_map(to_work->map_ref, hashval, name);
  } else {
    pthread_trylock_buffer(&to_work->map_ref[idx].mutex);
    to_work->map_ref[idx].count++;
    pthread_mutex_unlock(&to_work->map_ref[idx].mutex);
  }
  pthread_mutex_unlock(&to_work->mut);
}

void print_map(HashEntry *map, int n) {
  int i;
  for (i = 0; i < n; i++)
    if (map[i].in_use == true)
      printf("city: %s, count %i\n", map[i].key, map[i].count);
}