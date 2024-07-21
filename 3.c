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
  as well
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256       // max line size to read in
#define WORD_SIZE 100      // max size of city name in chars
#define MAX_ENTRIES 100000 // Total number of allowed city entries
#define MAX_KEYS 5         // order of our BTree

typedef struct {
  bool is_leaf;
  unsigned no_children;
  unsigned children[];
  unsigned no_keys;
  unsigned keys[];
} Node;
typedef struct {
  Node nodes[MAX_ENTRIES / 2];
  unsigned root_index;
  unsigned next_free_index;
} BTree;
typedef struct {
  char key[WORD_SIZE]; // Name of the city from input
  unsigned count;
  float min, max;
  double sum;
  unsigned value; // corresponds to idx of Node in BTree array
  bool in_use;    // true if entry corresponds to BTree Node
} HashEntry;

/* Add node with temp value to tree, return idx of node */
unsigned add_to_listing(char *name, float temp, BTree *tree, HashEntry map[]);
/* Add new value to hashmap after adding to tree*/
void add_node(char *name, unsigned val, HashEntry map[]);
/* Find node of tree using hashmap*/
int find_node(char *name, HashEntry map[]);
float max(float a, float b);
float min(float a, float b);

int main(void) {
  HashEntry map[MAX_ENTRIES];
  BTree tree;
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
      idx = add_to_listing(name, temp, tree, map);
      add_node(name, idx, map);
    } else {
      // add value to node
    }
    cnt++;
  }
  printf("Completed computing stats for %d cities", cnt);
  return 0;
}

// Helper function used to create nodes for the BTree
// doesn't associate any key information
void create_node(BTree *tree) {
  unsigned next_free_index = tree->next_free_index++;
  if (next_free_index > MAX_ENTRIES) {
    fprintf(stderr, "Max nodes reached\n");
    exit(1);
  }
  Node node = tree->nodes[next_free_index];
  node.is_leaf = true;
  node.no_keys = 0;
}

// Recursive version of 'add_to_listing'
// Whereas we simply increase the index by one every time we
// create a new node in the BTree ex
// in addition, we recursively search for the node
unsigned add_to_listing(char *name, float temp, BTree *tree, HashEntry map[]) {
  unsigned idx;
  // There could be space on
  Node *root = &tree->nodes[tree->root_index];
  if (root->no_keys == MAX_KEYS) {
    int newRootIndex = create_node(tree);
    tree->rootIndex = newRootIndex;
    BTreeNode *newRoot = &tree->nodes[newRootIndex];
    newRoot->isLeaf = 0;
    newRoot->numKeys = 0;
    newRoot->children[0] = root - tree->nodes;
    split_child(tree, newRootIndex, 0);
    tree_insert(tree, newRootIndex, key);
  } else {
    tree_insert(tree, tree->rootIndex, key);
  }
}
return idx;
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
  while (map[hashval].in_use == true) {
    long next = hashval + 1;
    if (next < hashval) {
      fprintf(stderr, "Overflow detected while adding hash node\n");
      exit(1);
    }
  }
  hashval++;
  strcpy(map[hashval].key, name);
  map[hashval].value = val;
  map[hashval].in_use = true;
}

// Calculation and sort helpers

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
