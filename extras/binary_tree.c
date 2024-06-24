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

struct tnode *insert_node(struct tnode *head, struct tnode *node) {
  if (head == NULL) {
    head = node;
  }
  if (node->val < head->val) {
    head->left = insert_node(head->left, node);
  } else if (node->val > head->val) {
    head->right = insert_node(head->right, node);
  }
  return head;
}

struct tnode *find_node(struct tnode *head, int val) {
  if (head == NULL || head->val == val)
    return head;
  if (val < head->val)
    return find_node(head->left, val);
  else
    return find_node(head->right, val);
}

void print_node(struct tnode *head) {
  if (head != NULL) {
    print_node(head->left);
    printf("Val: %d\n", head->val);
    print_node(head->right);
  }
}

int main(void) {
  struct tnode *root = create_node(9);
  struct tnode *n = create_node(19);
  struct tnode *a = create_node(6);
  struct tnode *v = create_node(22);
  struct tnode *t = create_node(1);

  assert(root->val == 9);
  assert(n->val == 19);

  insert_node(root, n);
  insert_node(root, a);
  insert_node(root, v);
  insert_node(root, t);

  assert(root->right->val == 19);
  assert(root->right->right->val == 22);
  assert(root->left->val == 6);

  a = find_node(root, 1);
  assert(a->val == 1);

  print_node(root);

  free(a);
  free(v);
  free(n);
  free(root);
}