/* 
    A take on the baseline approach by using a binary tree instead 
    of an array
*/
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NODE_MAX 43690
#define BUF_SIZE 1024 
#define WORD_SIZE 256

typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
  struct node *left;
  struct node *right;
} node;

struct node *create_node(int val) {
  struct node *new;
  new = malloc(sizeof(struct node));
  if (new == NULL) {
    fprintf(stderr, "malloc error\n");
    exit(1);
  }
  new->sum = val;
  new->count = 0;
  new->left = NULL;
  new->right = NULL;
  return new;
}

/* 
  As I iterate line-by-line
  I want to check and see if the city is listed in the tree
  - if it is, i want to add its measurements to its struct
  - else i want to create a struct for the city and add it's info

*/

// O(logn) node insertion
struct node *insert_node(struct node *head, struct node *node) {
  int res; 
  if (head == NULL || (res = strcmp(head->name, node->name)) == 0) {
    head = node;
  }
  if (strcmp(head->name, const char *s2)) {
    head->left = insert_node(head->left, node);
  } else ) {
    head->right = insert_node(head->right, node);
  }
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
void print_node(struct node *head) {
  if (head != NULL) {
    print_node(head->left);
    printf("Val: %d\n", head->val);
    print_node(head->right);
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

static int cmp(const void *ptr_a, const void *ptr_b) {
  return strcmp(((const node *)ptr_a)->name, ((const node *)ptr_b)->name);
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
    if (1) {
      array[i].count++;
      array[i].max = max(array[i].max, temp);
      array[i].min = min(array[i].min, temp);
      array[i].sum += temp;
    } else {
      strcpy(array[cur].name, name);
      array[cur].count = 0;
      array[cur].max = temp;
      array[cur].min = temp;
      array[cur].sum = temp;
      cur++;
    }
  }
  qsort(array, (size_t)cur, sizeof(*array), cmp);
  printf("{");
  for (i = 0; i < cur; i++) {
    printf("%s=%.1f/%.1f/%.1f%s", array[i].name, array[i].min,
           array[i].sum / array[i].count, array[i].max, i < cur - 1 ? ", " : "");
  }
  printf("}\n");
  return 0;
}

