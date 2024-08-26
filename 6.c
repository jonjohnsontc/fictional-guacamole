/*

I'm thinking that leveraging mmap as well as threads would allow me to better
shape the problem for threads.

I think to parallelize the problem better, I'd have to read the file in parallel
and operate using `nthreads` number of streams to the file. If I mmap the input
file, I can just copy `nthreads` number of pointers and have them operate on
contiguous regions of it.

Much of this is copied from Github user Danny van Kooten's analyze.c
implementation in terms of structure, and how data is processed in each thread
https://github.com/dannyvankooten/1brc/blob/main/analyze.c

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
#define MAX_THREADS 24 // TODO: capture num of threads during compilation
#define MAX_ENTRIES 100000
#define HASHMAP_CAPACITY 16384
#define HASHMAP_INDEX(h) (h & (HASHMAP_CAPACITY - 1))
#define MEASUREMENTS_FILE "./measurements_1b.txt"
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

float min(float a, float b);
float max(float a, float b);
int cmp(const void *ptr_a, const void *ptr_b);
unsigned int *get_key(Group *row_grouping, char *name);
void *process_rows(void *data);

int main(int argc, char *argv[]) {
  int fd, num_threads;
  char *addr;
  size_t size;
  fd = open(MEASUREMENTS_FILE, O_RDONLY);
  if (fd == -1)
    errno_abort("file open");

  size = lseek(fd, 0, SEEK_END);
  if (size == -1)
    errno_abort("lseek");

  addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (*addr == -1)
    errno_abort("mmap");

  num_threads = sysconf(_SC_NPROCESSORS_CONF) / 2;
  if (num_threads == -1)
    errno_abort("sysconf");
  if (num_threads > MAX_THREADS)
    num_threads = MAX_THREADS;

  // Calculate results
  pthread_t workers[MAX_THREADS];
  chunk_size = (size / (num_threads * 2));
  chunk_count = (size / chunk_size - 1) + 1;
  for (size_t i = 0; i < num_threads; i++) {
    pthread_create(&workers[i], NULL, process_rows, addr);
  }

  // wait for all threads to finish
  Group *results[MAX_THREADS];
  for (size_t i = 0; i < num_threads; i++) {
    int res = pthread_join(workers[i], (void *)&results[i]);
    if (res != 0)
      err_abort(res, "pthread_join");
  }

  munmap((void *)addr, size);

  // merge results
  Group *combined = results[0];
  for (size_t i = 0; i < num_threads; i++) {
    for (unsigned int j = 0; j < results[i]->size; j++) {
      Node *b = &results[i]->rows[j];
      unsigned int *hm_entry = get_key(combined, b->name);
      unsigned int c = *hm_entry;
      if (c == 0) {
        c = combined->size++;
        *hm_entry = c;
        strcpy(combined->rows[c].name, b->name);
      }
      combined->rows[c].count += b->count;
      combined->rows[c].sum += b->sum;
      combined->rows[c].min = min(combined->rows[c].min, b->min);
      combined->rows[c].max = max(combined->rows[c].max, b->max);
    }
  }

  qsort(combined->rows, combined->size, sizeof(*combined->rows), cmp);
  printf("{");
  for (unsigned int i = 0; i < combined->size; i++) {
    printf("%s=%.1f/%.1f/%.1f%s", combined->rows[i].name, combined->rows[i].min,
           combined->rows[i].sum / combined->rows[i].count,
           combined->rows[i].max, ", ");
  }
  printf("}\n");
  return 0;
}

void *process_rows(void *_data) {
  char *data = (char *)_data;

  Group *results = malloc(sizeof(*results));
  if (results == NULL)
    err_abort(ENOMEM, "malloc");

  results->size = 0;
  memset(results->map, 0, HASHMAP_CAPACITY * sizeof(*results->map));
  memset(results->rows, 0, MAX_ENTRIES * sizeof(*results->rows));

  // process data until done
  while (1) {
    const unsigned int chunk = chunk_selector++;
    if (chunk >= chunk_count)
      break;

    size_t chunk_start = chunk * chunk_size;
    size_t chunk_end = chunk_start + chunk_size;

    const char *start =
        chunk_start > 0
            ? (char *)memchr(&data[chunk_start], '\n', chunk_size) + 1
            : &data[chunk_start];

    // find pointer to last newline char, assumes file ends in a newline
    const char *end = (char *)memchr(&data[chunk_end], '\n', chunk_size) + 1;

    while (start < end) {
      const char *line_start = start;

      // let's hash the city name until we find the delimeter ';'
      unsigned int len = 0;
      unsigned int hash = 5381;
      while (*start != ';') {
        hash = ((hash << 5) + hash) + *start++;
        len++;
      }

      // advance past ';'
      start++;

      // parse the temperature value
      unsigned int temp_len = 0;
      char temp_container[10];
      while (*start != '\n') {
        temp_container[temp_len++] = *start++;
      }

      // NOTE: first time using this, last time used atof
      float d = strtod(temp_container, NULL);
      if (d == 0 && errno != 0)
        err_abort(errno, "strtod");

      // skip past newline '\n'
      start++;

      // try and find name in hashmap
      unsigned int *loc = &results->map[HASHMAP_INDEX(hash)];
      while (*loc > 0 &&
             memcmp(results->rows[*loc].name, line_start, len) != 0) {
        hash += 1;
        loc = &results->map[HASHMAP_INDEX(hash)];
      }

      // if unseen (0) we make a new entry
      if (*loc == 0) {
        *loc = results->size;
        memcpy(results->rows[*loc].name, line_start, len);
        results->size++;
      }

      // existing entries
      results->rows[*loc].count += 1;
      results->rows[*loc].min = min(results->rows[*loc].min, d);
      results->rows[*loc].max = max(results->rows[*loc].max, d);
      results->rows[*loc].sum += d;
    }
  }
  return (void *)results;
}

unsigned int *get_key(Group *row_grouping, char *name) {
  unsigned int hash = 5381;
  unsigned int len = 0;
  char *cur = name;
  while (*cur != '\0') {
    hash = ((hash << 5) + hash) + *cur++;
    len++;
  }

  unsigned int *c = &row_grouping->map[HASHMAP_INDEX(hash)];
  while (*c > 0 && memcmp(row_grouping->rows[*c].name, name, len) != 0) {
    hash++;
    c = &row_grouping->map[HASHMAP_INDEX(hash)];
  }
  return c;
}

float max(float a, float b) {
  if (a > b)
    return a;
  else
    return b;
}

float min(float a, float b) {
  if (a < b)
    return a;
  else
    return b;
}

int cmp(const void *ptr_a, const void *ptr_b) {
  return strcmp(((Node *)ptr_a)->name, ((Node *)ptr_b)->name);
}