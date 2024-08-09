/*
  Trying to build out a thread pool, and using
  https://nachtimwald.com/2019/04/12/thread-pool-in-c/ as a guide
*/
// HEADER FILE | INTERFACE
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
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
  tm->thread_cnt++;
  pthread_cond_signal(&(tm->working_cond));
  pthread_mutex_unlock(&(tm->work_mutex));
  return NULL;
}
