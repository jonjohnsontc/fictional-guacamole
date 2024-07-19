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
  unsigned value;
  bool in_use;
} HashEntry;

/* Add node with temp value to tree, return idx of node */
unsigned add_to_tree(char *name, float temp, Node tree[]);
/* Add new value to hashmap after adding to tree*/
void add_node(char *name, unsigned val, HashEntry map[]);
/* Find node of tree using hashmap*/
unsigned find_node(char *name, HashEntry map[]);
float max(float a, float b);
float min(float a, float b);

int main(void) {
  // we leverage GNU array extensions to initialize all values
  static Node tree[MAX_ENTRIES * 4 + 1] = {
      [0 ...(MAX_ENTRIES)] = {"", 0, 0.0, 0.0, 0.0, true}};
  static HashEntry map[MAX_ENTRIES] = {
      [0 ...(MAX_ENTRIES - 1)] = {"", 0, false}};
  char buffer[BUF_SIZE];
  char name[WORD_SIZE];
  unsigned idx, cnt;
  float temp;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    sscanf(buffer, "%[^;];%f", name, &temp);
    if ((idx = find_node(name, map)) == -1) {
      // create a new node for the city
      idx = add_to_tree(name, temp, tree);
      add_node(name, idx, map);
    } else {
      // add value to node
      tree[idx].count++;
      tree[idx].max = max(temp, tree[idx].max);
      tree[idx].min = min(temp, tree[idx].min);
      tree[idx].sum += temp;
    }
    cnt++;
  }
  return 0;
}

// Recursive version of 'add_to_tree'
int add_to_tree_r(char *name, float temp, Node tree[], unsigned *idx) {
  if (*idx > MAX_ENTRIES * 4) {
    fprintf(stderr, "Too many entries, quitting\n");
    exit(1);
  }
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

unsigned add_to_tree(char *name, float temp, Node tree[]) {
  unsigned idx = 1;
  return add_to_tree_r(name, temp, tree, &idx);
}

// djb2 hash function - found via http://www.cse.yorku.ca/~oz/hash.html
// (https://stackoverflow.com/questions/7666509/hash-function-for-string)
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash % MAX_ENTRIES;
}

unsigned find_node(char *name, HashEntry map[]) {
  long hashval = hash(name);
  while (map[hashval].in_use == true) {
    if (strcmp(map[hashval].key, name) == 0) {
      return map[hashval].value;
    } else {
      hashval++;
    }
  }
  return -1;
}

void add_node(char *name, unsigned val, HashEntry map[]) {
  long hashval = hash(name);
  while (map[hashval].in_use == true)
    hashval++;
  strcpy(map[hashval].key, name);
  map[hashval].value = val;
  map[hashval].in_use = true;
}

float max(float a, float b) {
  if (a > b)
    return a;
  return b;
}

float min(float a, float b) {
  if (a < b)
    return a;
  return b;
}
