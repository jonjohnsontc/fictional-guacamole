/*
  Identical to the tree solution (2tc), except we avoid calling malloc by
  keeping every node in the binary tree within an array
*/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 256
#define WORD_SIZE 100
#define MAX_ENTRIES 100000

typedef struct {
  char name[WORD_SIZE];
  unsigned count;
  float min, max;
  double sum;
  bool new; // true if Node doesn't belong to anything
} Node;
typedef struct {
  char key[WORD_SIZE];
  int value;
  bool in_use;
} HashEntry;

/* Add node with temp value to tree, return idx of node */
int add_to_tree(char *name, float temp, Node tree[]);
/* Find node of tree using hashmap*/
int find_node(char *name, HashEntry map[]);

int main(void) {
  static Node tree[MAX_ENTRIES + 1] = {
      [0 ...(MAX_ENTRIES)] = {"", 0, 0.0, 0.0, 0.0, true}};
  static HashEntry map[MAX_ENTRIES] = {
      [0 ...(MAX_ENTRIES - 1)] = {"", 0, false}};
  char buffer[BUF_SIZE];
  char name[WORD_SIZE];
  float temp;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    sscanf(buffer, "%[^;];%f", name, &temp);
  }
  printf("Hello trees!\n");
  printf("Here's the first one - %s val %f\n", tree[0].name, tree[0].min);
  add_to_tree("Fred", 22.8, tree);
  add_to_tree("Bill", 12.3, tree);
  find_node("Fred", map);
  printf("Here's the second one after being modified: - %s min %f\n",
         tree[2].name, tree[2].min);
  return 0;
}

// Recursive version of 'add_to_tree'
int add_to_tree_r(char *name, float temp, Node tree[], unsigned *idx) {
  if (tree[*idx].new) {
    strcpy(tree[*idx].name, name);
    tree[*idx].min = temp;
    tree[*idx].max = temp;
    tree[*idx].sum = temp;
    tree[*idx].count = 1;
    tree[*idx].new = false;
    return *idx;
  } else if (strcmp(tree[*idx].name, name) > 0)
    *idx = *idx * 2;
  else
    *idx = *idx * 2 + 1;
  return add_to_tree_r(name, temp, tree, idx);
}

int add_to_tree(char *name, float temp, Node tree[]) {
  unsigned idx = 1;
  return add_to_tree_r(name, temp, tree, &idx);
}

int find_node(char *name, HashEntry map[]) {
  int res = -1;
  return res;
}
