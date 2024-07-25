/*
    A take on the baseline approach by using a binary tree instead
    of an array. Still executed in a single thread. I'm also adding
    a hashmap to speed up lookup to O(1).
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1024
#define WORD_SIZE 256
#define MAX_ENTRIES 100000

typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
  struct node *left;
  struct node *right;
} node;
typedef struct {
  char key[WORD_SIZE]; // supposed to be the same as the corresponding
                       // City.name, only used for debugging now
  bool in_use;
  struct node *node;
} HashEntry;

struct node *create_node(char *name, int val) {
  struct node *new;
  new = malloc(sizeof(struct node));
  if (new == NULL) {
    fprintf(stderr, "malloc error\n");
    exit(1);
  }
  strcpy(new->name, name);
  new->sum = val;
  new->count = 0;
  new->left = NULL;
  new->right = NULL;
  return new;
}

void free_tree(node *n) {
  if (n != NULL) {
    free_tree(n->right);
    free_tree(n->left);
    free(n);
  }
}

/*
  As I iterate line-by-line

  I want to check and see if the city is listed in the tree
  - if it is, i want to add its measurements to its struct
  - else i want to create a struct for the city and add it's info

*/

// djb2 hash function - found via http://www.cse.yorku.ca/~oz/hash.html
// (https://stackoverflow.com/questions/7666509/hash-function-for-string)
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash % MAX_ENTRIES;
}

// O(logn) node insertion
struct node *insert_node(struct node *head, struct node *node) {
  if (head == NULL) {
    return node;
  }
  if (strcmp(head->name, node->name) > 0) {
    head->left = insert_node(head->left, node);
  } else {
    head->right = insert_node(head->right, node);
  }
  return head;
}

// O(logn) search for node
// struct node *find_node(struct node *head, char *name) {
//   int res;
//   if (head == NULL)
//     return head;
//   if ((res = strcmp(head->name, name)) == 0)
//     return head;
//   if (res < 0)
//     return find_node(head->right, name);
//   else
//     return find_node(head->left, name);
// }

// Best case O(1) access to city values
long get_map_index(char *name, HashEntry map[]) {
  long hashval = hash(name);
  while (map[hashval].in_use == true) {
    if (strcmp(map[hashval].key, name) == 0) {
      return hashval;
    } else {
      hashval++;
    }
  }
  return -1;
}

struct node *find_node(HashEntry map[], struct node *head, char *name) {
  int idx;
  if ((idx = get_map_index(name, map)) == -1)
    return NULL;
  else
    return map[idx].node;
}

// Prints tree in ascending order
void print_node(struct node *head, unsigned *remaining) {
  if (head != NULL) {
    print_node(head->left, &(*remaining--));
    printf("%s=%.1f/%.1f/%.1f%s", head->name, head->min,
           head->sum / head->count, head->max, ", ");
    print_node(head->right, &(*remaining--));
  }
}

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

void add_to_map(HashEntry map[], node *node, long hashval, char *name) {
  while (map[hashval].in_use == true) {
    hashval++;
    if (hashval > MAX_ENTRIES) {
      fprintf(stderr, "Error: Too many hashmap entries\n");
      exit(1);
    }
  }
  strcpy(map[hashval].key, name);
  map[hashval].node = node;
  map[hashval].in_use = true;
}

// void add_node(char *name, unsigned val, HashEntry map[]) {
//   long hashval = hash(name);
//   while (map[hashval].in_use == true) {
//     long next = hashval + 1;
//     if (next < hashval) {
//       fprintf(stderr, "Overflow detected while adding hash node\n");
//       exit(1);
//     }
//   }
//   hashval++;
//   strcpy(map[hashval].key, name);
//   map[hashval].city_idx = val;
//   map[hashval].in_use = true;
// }

int main(void) {
  char buf[BUF_SIZE];
  char name[WORD_SIZE];
  static HashEntry map[MAX_ENTRIES];
  node *r = NULL, *n = NULL, *f = NULL;
  float temp;
  long hashval;
  unsigned cur = 0;
  FILE *file = fopen("./measurements_100m.txt", "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    sscanf(buf, "%[^;];%f", name, &temp);
    if ((f = find_node(map, r, name)) != NULL) {
      f->count++;
      f->max = max(f->max, temp);
      f->min = min(f->min, temp);
      f->sum += temp;
    } else {
      hashval = hash(name);
      n = create_node(name, temp);
      add_to_map(map, n, hashval, name);
      r = insert_node(r, n);
      cur++;
    }
  }
  printf("{");
  print_node(r, &cur);
  printf("}\n");
  free_tree(r);
  return 0;
}
