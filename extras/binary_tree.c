/*
    After realizing I was having some trouble coming up with a way
    to use a binary tree effectively when iterating through rows in
    the 1brc, I decided to try implementing a simple binary tree to
   re-familiarize myself
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
struct tnode {
  int val;
  int count;
  struct tnode *left;
  struct tnode *right;
};

struct tnode *create_node(int val) {
  struct tnode *new;
  new = malloc(sizeof(struct tnode));
  if (new == NULL) {
    fprintf(stderr, "malloc error\n");
    exit(1);
  }
  new->val = val;
  new->count = 0;
  new->left = NULL;
  new->right = NULL;
  return new;
}

void insert_node(struct tnode *head, struct tnode *node) {
  struct tnode *t = NULL;
  while (head != NULL) {
    t = head;
    if (node->val < head->val) {
      head = head->left;
    }
  }
}

int main(void) {
  struct tnode *t = create_node(9);
  struct tnode *n = create_node(19);

  assert(t->val == 9);
  assert(n->val == 19);


}