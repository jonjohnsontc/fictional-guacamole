/*

  Asked ChatGPT to give me a Binary Tree that leverages an array / doesn't do
  dynamic memory allocation, and it looks like this is it.

*/
#include <stdio.h>
#include <stdlib.h>

#define MAX_KEYS 3
#define MAX_CHILDREN (MAX_KEYS + 1)
#define TREE_SIZE 100

typedef struct {
  int keys[MAX_KEYS];
  int numKeys;
  int children[MAX_CHILDREN];
  int isLeaf;
} BTreeNode;

typedef struct {
  BTreeNode nodes[TREE_SIZE];
  int rootIndex;
  int nextFreeIndex;
} BTree;

void initializeBTree(BTree *tree) {
  tree->rootIndex = 0;
  tree->nextFreeIndex = 1;
  tree->nodes[0].isLeaf = 1;
  tree->nodes[0].numKeys = 0;
}

int createNode(BTree *tree) {
  int index = tree->nextFreeIndex++;
  tree->nodes[index].isLeaf = 1;
  tree->nodes[index].numKeys = 0;
  return index;
}

void splitChild(BTree *tree, int parentIndex, int childIndex);

void insertNonFull(BTree *tree, int index, int key) {
  // starts at either a new root or the current root
  BTreeNode *node = &tree->nodes[index];
  int i = node->numKeys - 1;
  // all nodes start out as leaf nodes
  if (node->isLeaf) {
    // starting from the index passed
    // keep shifting them over to the right until you
    // run into one that is <= the key
    // (basically insert it in order)
    while (i >= 0 && node->keys[i] > key) {
      node->keys[i + 1] = node->keys[i];
      i--;
    }
    node->keys[i + 1] = key;
    node->numKeys++;
  } else {
    // go through the node until you hit a key that is == or
    // less than the key
    while (i >= 0 && node->keys[i] > key) {
      i--;
    }
    i++;
    // If the child at max keys, we'll need to split it
    if (tree->nodes[node->children[i]].numKeys == MAX_KEYS) {
      splitChild(tree, index, i);
      if (node->keys[i] < key) {
        i++;
      }
    }
    // insert the key in order to the child
    // will recursively iterate to the leaf node
    insertNonFull(tree, node->children[i], key);
  }
}

void splitChild(BTree *tree, int parentIndex, int childIndex) {
  BTreeNode *parent = &tree->nodes[parentIndex];
  BTreeNode *child = &tree->nodes[parent->children[childIndex]];
  // we first create a new node for the tree
  int newIndex = createNode(tree);
  BTreeNode *newNode = &tree->nodes[newIndex];
  newNode->isLeaf = child->isLeaf;
  newNode->numKeys = MAX_KEYS / 2;

  // assign keys from the halfway point and up of
  // the child's keys to the new node
  for (int j = 0; j < MAX_KEYS / 2; j++) {
    newNode->keys[j] = child->keys[j + MAX_KEYS / 2 + 1];
  }
  // if the child is not a leaf, we also assign
  // children from it's halfway mark and up to the new node
  if (!child->isLeaf) {
    for (int j = 0; j < MAX_KEYS / 2 + 1; j++) {
      newNode->children[j] = child->children[j + MAX_KEYS / 2 + 1];
    }
  }
  // then cut the child's number of keys in half
  child->numKeys = MAX_KEYS / 2;

  // move the children in the parent node to the right
  // until we get to the the node after the child index, and set
  // our new node to this value.
  for (int j = parent->numKeys; j >= childIndex + 1; j--) {
    parent->children[j + 1] = parent->children[j];
  }
  parent->children[childIndex + 1] = newIndex;

  for (int j = parent->numKeys - 1; j >= childIndex; j--) {
    parent->keys[j + 1] = parent->keys[j];
  }
  parent->keys[childIndex] = child->keys[MAX_KEYS / 2];
  parent->numKeys++;
}

void insert(BTree *tree, int key) {
  BTreeNode *root = &tree->nodes[tree->rootIndex];
  if (root->numKeys == MAX_KEYS) {
    int newRootIndex = createNode(tree);
    tree->rootIndex = newRootIndex;
    BTreeNode *newRoot = &tree->nodes[newRootIndex];
    newRoot->isLeaf = 0;
    newRoot->numKeys = 0;
    newRoot->children[0] = root - tree->nodes;
    splitChild(tree, newRootIndex, 0);
    insertNonFull(tree, newRootIndex, key);
  } else {
    insertNonFull(tree, tree->rootIndex, key);
  }
}

void printTree(BTree *tree, int index, int depth) {
  BTreeNode *node = &tree->nodes[index];
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  for (int i = 0; i < node->numKeys; i++) {
    printf("%d ", node->keys[i]);
  }
  printf("\n");
  if (!node->isLeaf) {
    for (int i = 0; i <= node->numKeys; i++) {
      printTree(tree, node->children[i], depth + 1);
    }
  }
}

int main(void) {
  BTree tree;
  initializeBTree(&tree);

  insert(&tree, 10);
  insert(&tree, 20);
  insert(&tree, 5);
  insert(&tree, 6);
  insert(&tree, 12);
  insert(&tree, 30);
  insert(&tree, 7);
  insert(&tree, 17);

  printTree(&tree, tree.rootIndex, 0);

  return 0;
}
