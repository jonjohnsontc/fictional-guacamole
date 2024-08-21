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
- start each thread with function that operates over its memory region using a
shared hashmap with mutex locked keys
- compute and print final result in main thread

*/
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

// Holds data in a binary tree
typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
  struct node *left;
  struct node *right;
  pthread_mutex_t mutex;
} node;

typedef struct {
  char key[WORD_SIZE];
  bool in_use;
  struct node *node;
  pthread_mutex_t mutex;
} HashEntry;
