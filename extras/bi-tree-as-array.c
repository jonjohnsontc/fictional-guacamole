/*
  I want to setup a binary tree using an array as the backing structure.
  as opposed to allocating memory and pointing to new stucts as I need to create
  them
*/
#include <math.h>
#include <stdio.h>
#define WORD_SIZE 256
#define MAX_ENTRIES 100000

typedef struct {
  char name[WORD_SIZE];
  int val;
} Node;

// I need add and find operations
int add_to_tree(char *name, float temp);
int find_node(char *name);

int main(void) {
  // experimenting with statically allocating some dummy info up front
  static Node tree[MAX_ENTRIES + 1] = {[0 ...(MAX_ENTRIES)] = {"", NAN}};
  printf("Hello trees!\n");
  printf("Here's the first one - %s val %d\n", tree[0].name, tree[0].val);
  return 0;
}

// add's name and temperature to tree in lexigraphical order
int add_to_tree(char *name, float temp) {
  int idx;
  return idx;
}