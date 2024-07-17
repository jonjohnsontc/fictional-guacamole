/*
  I want to setup a binary tree using an array as the backing structure.
  as opposed to allocating memory and pointing to new stucts as I need to create
  them
*/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define WORD_SIZE 256
#define MAX_ENTRIES 100000

typedef struct {
  char name[WORD_SIZE];
  float val;
  bool new; // true if Node doesn't belong to anything
} Node;

// I need add and find operations
int add_to_tree(char *name, float temp, Node tree[]);

int main(void) {
  // experimenting with statically allocating some dummy info
  static Node tree[MAX_ENTRIES + 1] = {[0 ...(MAX_ENTRIES)] = {"", 0, true}};
  printf("Hello trees!\n");
  printf("Here's the first one - %s val %f\n", tree[0].name, tree[0].val);
  add_to_tree("Fred", 22.8, tree);
  add_to_tree("Bill", 12.3, tree);
  return 0;
}

// Recursive version of 'add_to_tree'
int add_to_tree_r(char *name, float temp, Node tree[], unsigned *idx) {
  if (tree[*idx]->new) {
    strcpy(tree[*idx]->name, name);
    tree[*idx]->val = temp;
    tree[*idx]->new = false;
    return *idx;
  } else if (strcmp(tree[*idx]->name, name) > 0)
    *idx = *idx * 2;
  else
    *idx = *idx * 2 + 1;
  return add_to_tree_r(name, temp, tree, idx);
}

int add_to_tree(char *name, float temp, Node *tree[]) {
  unsigned idx = 1;
  return add_to_tree_r(name, temp, tree, &idx);
}