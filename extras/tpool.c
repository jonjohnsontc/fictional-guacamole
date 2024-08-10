/*
  Trying to build out a thread pool, and using
  https://nachtimwald.com/2019/04/12/thread-pool-in-c/ as a guide
*/
// HEADER FILE | INTERFACE
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void *arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);
// END HEADER FILE
typedef struct tpool_work {
  thread_func_t func;
  void *arg;
  struct tpool_work *next;
} tpool_work_t;

struct tpool {
  tpool_work_t *work_first;    // pop
  tpool_work_t *work_last;     // pop-last
  pthread_mutex_t work_mutex;  // controls all locking for pool
  pthread_cond_t work_cond;    // signals if there's work to be processed
  pthread_cond_t working_cond; // if there are threads processing work
  size_t working_cnt;          // number of threads working
  size_t thread_cnt;           // total thread count
  bool stop;                   // whether to kill
};

static tpool_work_t *tpool_work_create(thread_func_t func, void *arg) {
  tpool_work_t *work;
  if (func == NULL)
    return NULL;

  work = malloc(sizeof(*work));
  work->func = func;
  work->arg = arg;
  work->next = NULL;
  return work;
}

static void tpool_work_destroy(tpool_work_t *work) {
  if (work == NULL)
    return;
  free(work);
}

static tpool_work_t *tpool_work_get(tpool_t *tm) {
  tpool_work_t *work;

  if (tm == NULL)
    return NULL;
  work = tm->work_first;
  if (work->next == NULL) {
    tm->work_first = NULL;
    tm->work_last = NULL;
  } else {
    tm->work_first = work->next;
  }
  return work;
}

static void *tpool_worker(void *arg) {
  tpool_t *tm = arg;
  tpool_work_t *work;

  while (1) {
    pthread_mutex_lock(&(tm->work_mutex));
    while (tm->work_first == NULL && !tm->stop)
      pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

    if (tm->stop)
      break;

    work = tpool_work_get(tm);
    tm->working_cnt++;
    pthread_mutex_unlock(&(tm->work_mutex));

    if (work != NULL) {
      work->func(work->arg);
      tpool_work_destroy(work);
    }

    pthread_mutex_lock(&(tm->work_mutex));
    tm->working_cnt--;
    if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
      pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
  }
  tm->thread_cnt--;
  pthread_cond_signal(&(tm->working_cond));
  pthread_mutex_unlock(&(tm->work_mutex));
  return NULL;
}

tpool_t *tpool_create(size_t num) {
  tpool_t *tm;
  pthread_t thread;
  size_t i;

  if (num == 0)
    num = 2;

  tm = calloc(1, sizeof(*tm));
  tm->thread_cnt = num;

  pthread_mutex_init(&(tm->work_mutex), NULL);
  pthread_cond_init(&(tm->work_cond), NULL);
  pthread_cond_init(&(tm->working_cond), NULL);

  tm->work_first = NULL;
  tm->work_last = NULL;

  for (i = 0; i < num; i++) {
    pthread_create(&thread, NULL, tpool_worker, tm);
    pthread_detach(thread);
  }
  return tm;
}

void tpool_destroy(tpool_t *tm) {
  tpool_work_t *work;
  tpool_work_t *work2;

  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  work = tm->work_first;
  while (work != NULL) {
    work2 = work->next;
    tpool_work_destroy(work);
    work = work2;
  }

  tm->work_first = NULL;
  tm->stop = true;

  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  tpool_wait(tm);

  pthread_mutex_destroy(&(tm->work_mutex));
  pthread_cond_destroy(&(tm->work_cond));
  pthread_cond_destroy(&(tm->working_cond));
  free(tm);
}

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg) {
  tpool_work_t *work;

  if (tm == NULL)
    return false;

  work = tpool_work_create(func, arg);
  if (work == NULL)
    return false;

  pthread_mutex_lock(&(tm->work_mutex));
  if (tm->work_first == NULL) {
    tm->work_first = work;
    tm->work_last = tm->work_first;
  } else {
    tm->work_last->next = work;
    tm->work_last = work;
  }

  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));
  return true;
}

void tpool_wait(tpool_t *tm) {
  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  while (1) {
    if (tm->work_first != NULL || (!tm->stop && tm->working_cnt != 0) ||
        (tm->stop && tm->thread_cnt != 0)) {
      pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&(tm->work_mutex));
}

// TESTING
static const size_t num_threads = 4;
static const size_t num_items = 100;

void worker(void *arg) {
  int *val = arg;
  int old = *val;

  *val += 1000;
  printf("tid=%ld, old=%d, val=%d\n", pthread_self(), old, *val);
  if (*val % 2)
    usleep(100000);
}

int main(int argc, char **argv) {
  tpool_t *tm;
  int *vals;
  size_t i;

  tm = tpool_create(num_threads);
  vals = calloc(num_items, sizeof(*vals));

  for (i = 0; i < num_items; i++) {
    vals[i] = i;
    tpool_add_work(tm, worker, vals + i);
  }

  tpool_wait(tm);

  for (i = 0; i < num_items; i++) {
    printf("%d\n", vals[i]);
  }

  free(vals);
  tpool_destroy(tm);
  return 0;
}
