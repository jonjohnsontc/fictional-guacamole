/*

The same strategy as in 6, but instead of processing each temperature
as the row is read in, we keep a buffer of temperatures per city, and leverage
vector intrinsics to process the buffer once it hits a certain size.

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
#define CHUNK_SIZE 1024 
#define MAX_ENTRIES 100000
#define HASHMAP_CAPACITY 16384
#define HASHMAP_INDEX(h) (h & (HASHMAP_CAPACITY - 1))
#define MEASUREMENTS_FILE "./measurements_1m.txt"   // TODO: Change to 1b
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
  unsigned int buf_size;
  float buffer[CHUNK_SIZE];
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
static inline void to_string(char *buffer, Group *row_grouping);
static inline void compute_stats(Node *node);

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
  to_string(buffer, combined);
  printf(buffer);
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

      // existing entries
      results->rows[*loc].count += 1;
      results->rows[*loc].buf_size += 1;
      results->rows[*loc].buffer[results->rows[*loc].buf_size - 1] = d;
      if (results->rows[*loc].buf_size == CHUNK_SIZE) 
        compute_stats(&results->rows[*loc]);
    }

    // Process any remaining rows
    for (unsigned int i = 0; i < results->size - 1; i++)
      if (results->rows[i].buf_size > 0) 
        compute_stats(&results->rows[i]);
  }
  return (void *)results;
}

static inline unsigned int *get_key(Group *row_grouping, char *name) {
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

static inline void to_string(char *buffer, Group *row_grouping) {
  *buffer++ = '{';
  for (unsigned int i = 0; i < row_grouping->size; i++) {
    size_t n = (size_t)sprintf(
      buffer, "%s=%.1f/%.1f/%.1f%s", row_grouping->rows[i].name, row_grouping->rows[i].min,
           row_grouping->rows[i].sum / row_grouping->rows[i].count,
           row_grouping->rows[i].max, i < (row_grouping->size -1) ? ", " : "");

      buffer += n;
  }
  *buffer++ = '}';
  *buffer++ = '\n';
  *buffer++ = '\0';
}

static inline void compute_stats(Node *node) {
  __m128 vmin = _mm_set1_ps(node->min);
  __m128 vmax = _mm_set1_ps(node->max);
  __m128 vsum = _mm_setzero_ps();

  for (unsigned int i = 0; i < node->buf_size; i += 4) {
    __m128 vtemp = _mm_loadu_ps(&node->buffer[i]); // load next 4 floats
    vmin = _mm_min_ps(vmin, vtemp);
    vmax = _mm_max_ps(vmax, vtemp);
    vsum = _mm_add_ps(vsum, vtemp);  
  }

  // Horizontal reduction for min and max
  vmin = _mm_min_ps(vmin, _mm_shuffle_ps(vmin, vmin, _MM_SHUFFLE(2, 3, 0, 1)));
  vmin = _mm_min_ps(vmin, _mm_shuffle_ps(vmin, vmin, _MM_SHUFFLE(1, 0, 3, 2)));
  vmax = _mm_max_ps(vmax, _mm_shuffle_ps(vmax, vmax, _MM_SHUFFLE(2, 3, 0, 1)));
  vmax = _mm_max_ps(vmax, _mm_shuffle_ps(vmax, vmax, _MM_SHUFFLE(1, 0, 3, 2)));

  float min_temp, max_temp, sum_temp[4];
  _mm_store_ss(&min_temp, vmin);
  _mm_store_ss(&max_temp, vmax);
  _mm_storeu_ps(sum_temp, vsum);

  node->min = min(node->min, min_temp);
  node->max = max(node->max, max_temp);
  node->sum += sum_temp[0] + sum_temp[1] + sum_temp[2] + sum_temp[3];
  node->count += node->buf_size;

  // reset buffer count before exiting
  node->buf_size = 0;
}