/*
    2nd attempt at baseline
    After realizing I can't easily implement a hash map or binary tree,
    I'm just going to store the results in an array. At least I know how to
   search and add to one I think I'm going to try and calculate as I read in the
   cities/temps
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NODE_MAX 43690
#define BUF_SIZE 4096
#define WORD_SIZE 256

typedef struct node {
  char name[100];
  int count;
  float min;
  float max;
  float sum;
} node;
typedef node *table[NODE_MAX];

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

static int cmp(const void *ptr_a, const void *ptr_b) {
  return strcmp(((const node *)ptr_a)->name, ((const node *)ptr_b)->name);
}

node *new_node(char *name, float val) {
  node *n = malloc(sizeof(node));
  if (n == NULL) {
    fprintf(stderr, "malloc error\n");
    exit(1);
  }
  strcpy(n->name, name);
  n->max = val;
  n->min = val;
  n->sum = val;
  n->count = 1;
  return n;
}

int in_table(table t, int size, char *name) {
  int i;
  for (i = 0; i < size; i++) {
    if (strcmp(t[i]->name, name) == 0) {
      return i;
    }
  }
  return -1;
}

void add_to_table(table t, char *name, float val, int *idx) {
  int i;
  if ((i = in_table(t, *idx, name)) != -1) {
    t[i]->count++;
    t[i]->max = max(t[i]->max, val);
    t[i]->min = min(t[i]->min, val);
    t[i]->sum += val;
  } else {
    t[(*idx)++] = new_node(name, val);
  }
}

int main(void) {
  char buf[BUF_SIZE];
  char name[WORD_SIZE];
  table temps = {NULL};
  float temp;
  int cur = 0, i;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    sscanf(buf, "%[^;];%f", name, &temp);
    add_to_table(temps, name, temp, &cur);
  }
  qsort(temps, (size_t)cur, sizeof(char *), cmp);
  for (i = 0; i < cur; i++) {
    printf("%s=%.1f/%.1f/%.1f%s", temps[i]->name, temps[i]->min,
           temps[i]->sum / temps[i]->count, temps[i]->max, i < cur ? ", " : "");
  }
  return 0;
}
