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
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUF_SIZE 256
#define WORD_SIZE 100
#define MAX_THREADS 24
#define MAX_ENTRIES 100000
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

/*
  All of the hashmap related structures + functions defined here
  Give us constant access to city's in array that have already been added

*/
typedef struct {
  char key[WORD_SIZE];
  bool in_use;
  struct node *node;
} HashEntry;

void *process(void *arg);
int main(void) {
  static HashEntry map[MAX_ENTRIES];
  char buf[BUF_SIZE];
  pthread_t threads[MAX_THREADS];
  int status;
  // _SC_NPROCESSORS_ONLN is the number of currently available procs
  int num_threads = sysconf(_SC_NPROCESSORS_CONF) / 2;
  printf("Number of threads configured is %d\n", num_threads);
  // Create thread pool
  for (int i = 0; i < num_threads; i++) {
    status = pthread_create(&threads[i], NULL, process, NULL);
    if (status != 0)
      err_abort(status, "thread creation");
  }
  printf("All threads created\n");
  FILE *file = fopen(MEASUREMENTS_FILE, "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    // Enqueue line in work
  }

  // Print result after it's done
  // Destroy thread pool
  for (int i = 0; i < num_threads; i++)
    status = pthread_join(threads[i], NULL);
  printf("All threads completed\n");
  return 0;
}

void *process(void *arg) {
  sleep(1);
  return NULL;
}