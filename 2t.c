/*
    A take on the baseline approach by using a binary tree instead
    of an array
*/
#include <complex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1024
#define WORD_SIZE 256

typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
  struct node *left;
  struct node *right;
} node;

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

// O(logn) node insertion
struct node *insert_node(struct node *head, struct node *node) {
  if (head == NULL) {
    return node;
  }
  if (strcmp(head->name, node->name) < 0) {
    head->left = insert_node(head->left, node);
  } else {
    head->right = insert_node(head->right, node);
  }
  return head;
}

// O(logn) search for node
struct node *find_node(struct node *head, char *name) {
  int res;
  if (head == NULL || (res = strcmp(head->name, name)) == 0)
    return head;
  if (res < 0)
    return find_node(head->left, name);
  else
    return find_node(head->right, name);
}

// Prints tree in ascending order
void print_node(struct node *head, unsigned *remaining) {
  if (head != NULL) {
    print_node(head->left, &(*remaining--));
    printf("%s=%.1f/%.1f/%.1f%s", head->name, head->min,
           head->sum / head->count, head->max, *remaining != 0 ? ", " : "");
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

int main(void) {
  char buf[BUF_SIZE];
  char name[WORD_SIZE];
  node *r = NULL, *n = NULL;
  float temp;
  unsigned cur = 0;
  FILE *file = fopen("./measurements_1m.txt", "r");
  while (fgets(buf, sizeof(buf), file) != NULL) {
    sscanf(buf, "%[^;];%f", name, &temp);
    if (find_node(r, name) != NULL) {
      r->count++;
      r->max = max(r->max, temp);
      r->min = min(r->min, temp);
      r->sum += temp;
    } else {
      n = create_node(name, temp);
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
