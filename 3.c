/*
  Identical to the tree solution (2tc), except we avoid calling malloc by
  keeping every node in a Btree within an array

  TODO: Add BTree impl
  Right now this is going to segfault going through all of the cities, because
  the binary tree used does not balance itself

  We first statically allocate enough memory to hold a BTree and Hashmap
  referencing the entities in the btree. We hardcode the filename to read in,
  and begin reading the file line by line
  We split the line into city name and temperature values, and see if the city
  name is located in the cities hashmap. If so, we retrieve the city's reference
  number and use it to pull the city from the BTree. If it's not, we generate a
  new BTree node with the city's identity and add it to the BTree and hashmap.

  After retrieving this BTree node, we add the temperature reading to each
  of the summary stats that we can.

  After reading all lines, we print the contents of the BTree (cities) in
  alphabetical order along with their statistics. We compute the average
  temperature for each city as we iterate through them. This info is printed
  as well;
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
int find_node(char *name, HashEntry map[]);
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
  unsigned cnt = 0;
  int idx;
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
  printf("Completed computing stats for %d cities", cnt);
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

int find_node(char *name, HashEntry map[]) {
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
