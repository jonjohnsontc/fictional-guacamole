/*
  Identical to the tree solution (2tc), except we avoid calling malloc by
  keeping every node in a Btree within an array

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

  ---

  How will this be implemented?

  Maybe three data structures:
    - BTree (to hold order things should be printed in)
      - stores ref to Temp array
    - Temp array (to store temperature names & values)
      - stores refs to BTree and Hashmap
    - Hashmap (for O(1) lookup of cities)
      - stores ref to Temp array

  To add a new city:
    - Make sure that we have space in the temperature array
    - add temperature value to attrs
    - hash and add to hashmap
    - add to Btree

  To add to Btree
    - grab root node, see if its full
    - if it is split the root node
    - else insert the index of the city node in order based on it's name into
  the BTree
      - this step happens while recursively looking for a leaf node
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
  unsigned children[MAX_KEYS + 1];
  unsigned no_keys;
  unsigned keys[MAX_KEYS];
  unsigned city_idx;
} Node;
typedef struct {
  Node nodes[MAX_ENTRIES / 5];
  unsigned root_index;
  unsigned next_free_index;
} BTree;
typedef struct {
  char key[WORD_SIZE]; // supposed to be the same as the corresponding
                       // City.name, only used for debugging now
  unsigned city_idx;
  bool in_use;
} HashEntry;
typedef struct {
  char name[WORD_SIZE];
  unsigned count;
  float min, max;
  double sum;
  unsigned hash_idx; // idx in hashmap
  unsigned tree_idx; // idx in tree
} City;
typedef struct {
  unsigned count;
  City v[MAX_ENTRIES];
} Cities;

/* Add node with temp value to tree, return idx of node */
void add_to_listing(char *name, float temp, Cities *cities, BTree *tree,
                    HashEntry map[]);
/* Find node of tree using hashmap*/
long get_map_index(char *name, HashEntry map[]);
void print_cities(Cities *cities, BTree *tree);
float max(float a, float b);
float min(float a, float b);

int main(void) {
  static Cities cities;
  static HashEntry map[MAX_ENTRIES];
  static BTree tree;
  char buffer[BUF_SIZE];
  char name[WORD_SIZE];
  unsigned cnt = 0;
  int idx;
  float temp;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    sscanf(buffer, "%[^;];%f", name, &temp);
    if ((idx = get_map_index(name, map)) == -1) {
      // create a new node for the city
      add_to_listing(name, temp, &cities, &tree, map);
    } else {
      // add value to node
      City city = cities.v[idx];
      city.count++;
      city.max = max(city.max, temp);
      city.min = min(city.min, temp);
      city.sum += temp;
    }
    cnt++;
  }
  print_cities(&cities, &tree);
  printf("Completed computing stats for %d cities", cnt);
  return 0;
}

// Helper function used to create nodes for the BTree
// void  doesn't associate any key information
unsigned create_node(BTree *tree) {
  unsigned next_free_index = tree->next_free_index++;
  if (next_free_index > MAX_ENTRIES) {
    fprintf(stderr, "Max nodes reached\n");
    exit(1);
  }
  Node node = tree->nodes[next_free_index];
  node.is_leaf = true;
  node.no_keys = 0;
  return next_free_index;
}

void split_child(BTree *tree, Cities *cities, unsigned parent_idx,
                 unsigned child_idx) {
  Node *parent = &tree->nodes[parent_idx];
  Node *child = &tree->nodes[parent->children[child_idx]];
  City city;
  unsigned city_idx;
  // we first create a new node for the tree
  unsigned new_idx = create_node(tree);
  Node *new_node = &tree->nodes[new_idx];
  new_node->is_leaf = child->is_leaf;
  new_node->no_keys = MAX_KEYS / 2;

  // assign keys from the halfway point and up of
  // the child's keys to the new node
  for (int j = 0; j < MAX_KEYS / 2; j++) {
    new_node->keys[j] = city_idx = child->keys[j + MAX_KEYS / 2 + 1];
    // we also need to update the corresponding city so it points
    // to it's new BTree node
    city = cities->v[new_node->keys[j]];
    city.tree_idx = new_idx;
  }
  // if the child is not a leaf, we also assign
  // children from it's halfway mark and up to the new node
  if (!child->is_leaf) {
    for (int j = 0; j < MAX_KEYS / 2 + 1; j++) {
      new_node->children[j] = child->children[j + MAX_KEYS / 2 + 1];
    }
  }
  // then cut the child's number of keys in half
  child->no_keys = MAX_KEYS / 2;

  // move the children in the parent node to the right
  // until we get to the the node after the child index, and set
  // our new node to this value.
  for (int j = parent->no_keys; j >= child_idx + 1; j--) {
    parent->children[j + 1] = parent->children[j];
  }
  parent->children[child_idx + 1] = new_idx;

  // Set j to last index of parent keys, while j is greater than or
  // equal to the child_idx, shift each key to the right
  for (int j = parent->no_keys - 1; j >= child_idx; j--) {
    parent->keys[j + 1] = parent->keys[j];
  }
  // to fill the role of new comparison key,
  // the child_idx in the parent keys will now be
  // set to what was the middle of the child keys
  parent->keys[child_idx] = child->keys[MAX_KEYS / 2];
  parent->no_keys++;
};

int comp_keys(Cities *cities, unsigned key1, unsigned key2) {
  City city1 = cities->v[key1];
  City city2 = cities->v[key2];
  return strcmp(city1.name, city2.name);
}

void tree_insert(BTree *tree, Cities *cities, unsigned idx,
                 unsigned new_city_idx) {
  // starts at either a new root or the current root
  Node *node = &tree->nodes[idx];
  int i = node->no_keys - 1;
  // all nodes start out as leaf nodes
  if (node->is_leaf) {
    // starting from the index passed
    // keep shifting them over to the right until you
    // run into one that is <= the key
    // (basically insert it in order)
    // node->keys[i]
    // key

    while (i >= 0 && comp_keys(cities, node->keys[i], new_city_idx)) {
      node->keys[i + 1] = node->keys[i];
      i--;
    }
    node->keys[i + 1] = new_city_idx;
    node->no_keys++;
  } else {
    // go through the node until you hit a key that is == or
    // less than the key
    while (i >= 0 && comp_keys(cities, node->keys[i], new_city_idx)) {
      i--;
    }
    i++;
    // If the child at max keys, we'll need to split it
    if (tree->nodes[node->children[i]].no_keys == MAX_KEYS) {
      split_child(tree, cities, idx, i);
      if (comp_keys(cities, idx, node->keys[i])) {
        i++;
      }
    }
    // insert the key in order to the child
    // will recursively iterate to the leaf node
    tree_insert(tree, cities, node->children[i], new_city_idx);
  }
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

// right now i'm trying to combine the functionality of the insert function
// from the BTree example and the ability to add the city and all of it's
// information to the hash map
void add_to_listing(char *name, float temp, Cities *cities, BTree *tree,
                    HashEntry map[]) {
  // We already know that the entry does not exist
  // the next available index in the cities array will be
  // its count
  unsigned next_idx = cities->count;
  City new_city = cities->v[next_idx];
  strcpy(new_city.name, name);
  new_city.max = temp;
  new_city.min = temp;
  new_city.sum = temp;
  new_city.count = 1;

  // set the city up in our hashmap
  long hashval = hash(name);
  while (map[hashval].in_use == true) {
    // because of how I'm dealing with collisions (just incrementing)
    // I'm going to check and see if I am beyond the array bounds each
    // time we increment
    unsigned long next = hashval + 1;
    if (next > MAX_ENTRIES) {
      fprintf(stderr, "Overflow detected while adding hash node\n");
    }
  }

  // Here's where we deal with storing the node in a BTree, which helps us
  // preserve their alphabetical order and allows for in order printing
  // at the very end
  Node *root = &tree->nodes[tree->root_index];
  if (root->no_keys == MAX_KEYS) {
    unsigned new_root_idx = create_node(tree);
    tree->root_index = new_root_idx;
    Node *new_root = &tree->nodes[new_root_idx];
    new_root->is_leaf = 0;
    new_root->no_keys = 0;
    new_root->children[0] = root - tree->nodes;
    split_child(tree, cities, new_root_idx, 0);
    tree_insert(tree, cities, new_root_idx, next_idx);
  } else {
    tree_insert(tree, cities, tree->root_index, next_idx);
  }
}

// Best case O(1) access to city values
long get_map_index(char *name, HashEntry map[]) {
  long hashval = hash(name);
  while (map[hashval].in_use == true) {
    if (strcmp(map[hashval].key, name) == 0) {
      return map[hashval].city_idx;
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
  map[hashval].city_idx = val;
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

void print_cities(Cities *cities, BTree *tree) {
  unsigned root_idx = tree->root_index;
  printf("print_cities: Not Implemented\n");
}