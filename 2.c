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
  return strcmp((*(const node **)ptr_a)->name, (*(const node **)ptr_b)->name);
}

int in_array(node array[], int size, char *name) {
  int i;
    for (i = 0; i < size; i++) {
    if (strcmp(array[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int main(void) {
  char buf[BUF_SIZE];
  char name[WORD_SIZE];
  node array[NODE_MAX];
  float temp;
  int cur = 0, i;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    sscanf(buf, "%[^;];%f", name, &temp);
    int i;
    if ((i = in_array(array, cur, name))) {
      strcpy(array[i].name, name);
      array[i].count++;
      array[i].max = max(array[i].max, temp);
      array[i].min = min(array[i].min, temp);
      array[i].sum += temp;
    } else {
      array[i].count = 0;
      array[i].max = temp;
      array[i].min = temp;
      array[i].sum = temp;
    }
  }
  qsort(array, (size_t)cur, sizeof(array), cmp);
  printf("{");
  for (i = 0; i < cur; i++) {
    printf("%s=%.1f/%.1f/%.1f%s", array[i].name, array[i].min,
           array[i].sum / array[i].count, array[i].max, i < cur - 1 ? ", " : "");
  }
  printf("}\n");
  return 0;
}
