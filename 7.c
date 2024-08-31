/*

  Trying to add SIMD to my previous implementation

*/
#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
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
#define LANES 4 // How many operations we're going to try and do at once
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

static inline float min(float a, float b);
static inline float max(float a, float b);
static inline int cmp(const void *ptr_a, const void *ptr_b);
static inline unsigned int *get_key(Group *row_grouping, char *name);
static inline void *process_rows(void *data);
static inline void print_data(char *buffer, Group *row_groupbing);
static inline void x4_count(const unsigned int nodes[], Group *results);
static inline void x4_min(const unsigned int nodes[], Group *results);
static inline void x4_max(const unsigned int nodes[], Group *results);
static inline void x4_add(const unsigned int nodes[], Group *results);

int main(void) {
  int fd;
  char *addr;
  long size;
  fd = open(MEASUREMENTS_FILE, O_RDONLY);
  if (fd == -1)
    errno_abort("file open");

  size = lseek(fd, 0, SEEK_END);
  if (size == -1)
    errno_abort("lseek");

  addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (*addr == -1)
    errno_abort("mmap");

  // Calculate results
  pthread_t workers[NTHREADS];
  chunk_size = (size / (NTHREADS * 2));
  chunk_count = (size / chunk_size - 1) + 1;
  for (size_t i = 0; i < NTHREADS; i++) {
    pthread_create(&workers[i], NULL, process_rows, addr);
  }

  // wait for all threads to finish
  Group *results[NTHREADS];
  for (size_t i = 0; i < NTHREADS; i++) {
    int res = pthread_join(workers[i], (void *)&results[i]);
    if (res != 0)
      err_abort(res, "pthread_join");
  }

  munmap((void *)addr, size);

  // merge results
  Group *combined = results[0];
  for (size_t i = 0; i < NTHREADS; i++) {
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
  char buffer[16384];
  print_data(buffer, combined);
  return 0;
}

static inline void *process_rows(void *_data) {
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

    // I want some way of tracking whether or not I've found the number of
    // records necessary for the simd add
    unsigned int nodes[LANES] = {0, 0, 0, 0};
    float temps[LANES] = {0, 0, 0, 0};
    unsigned int node_loc = 0;
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
      float d = strtod(temp_container, NULL);
      if (d == 0 && errno != 0)
        errno_abort("strtod");

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

      nodes[node_loc++] = HASMAP_INDEX(hash);
      // if node_loc has exceeded the number of locations / lanes available
      // that means we can process all of the data
      if (node_loc == LANES) {
        x4_count(nodes, &results);
        x4_min(nodes, &results);
        x4_max(nodes, &results);
        x4_add(nodes, &results);
        // reset node location, we don't need to empty the array because
        // we won't touch it again until the nodes have been traversed once more
        node_loc = 0;
      } else
        continue;
    }
    return (void *)results;
  }

  static inline unsigned int *get_key(Group * row_grouping, char *name) {
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

  static inline float max(float a, float b) {
    if (a > b)
      return a;
    else
      return b;
  }

  static inline float min(float a, float b) {
    if (a < b)
      return a;
    else
      return b;
  }

  static inline int cmp(const void *ptr_a, const void *ptr_b) {
    return strcmp(((Node *)ptr_a)->name, ((Node *)ptr_b)->name);
  }

  static inline void print_data(char *buffer, Group *row_grouping) {
    *buffer++ = '{';
    for (unsigned int i = 0; i < row_grouping->size; i++) {
      size_t n = (size_t)sprintf(
          buffer, "%s=%.1f/%.1f/%.1f%s", row_grouping->rows[i].name,
          row_grouping->rows[i].min,
          row_grouping->rows[i].sum / row_grouping->rows[i].count,
          row_grouping->rows[i].max, i < (row_grouping->size - 1) ? ", " : "");

      buffer += n;
    }
    *buffer++ = '}';
    *buffer++ = '\n';
    *buffer++ = '\0';
  }

  static inline void x4_count(const unsigned int nodes[LANES], Group *results) {
    counts[0] = results->rows[nodes[0]].count++;
    counts[1] = results->rows[nodes[1]].count++;
    counts[2] = results->rows[nodes[2]].count++;
    counts[3] = results->rows[nodes[3]].count++;
  }

  static inline void x4_min(const unsigned int nodes[], Group *results) {
    float mins[LANES] = {
        results->rows[nodes[0]].min, results->rows[nodes[1]].min,
        results->rows[nodes[2]].min, results->rows[nodes[3]].min};
    __m128 loaded = _mm_loadu_ps((const float *)&mins);
  }
  static inline void x4_max(const unsigned int nodes[], Group *results) {}
  static inline void x4_add(const unsigned int nodes[], Group *results) {}
