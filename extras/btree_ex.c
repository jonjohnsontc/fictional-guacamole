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
    // starting from the right side of the keys
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
    while (i >= 0 && node->keys[i] > key) {
      i--;
    }
    i++;
    if (tree->nodes[node->children[i]].numKeys == MAX_KEYS) {
      splitChild(tree, index, i);
      if (node->keys[i] < key) {
        i++;
      }
    }
    insertNonFull(tree, node->children[i], key);
  }
}

void splitChild(BTree *tree, int parentIndex, int childIndex) {
  BTreeNode *parent = &tree->nodes[parentIndex];
  BTreeNode *child = &tree->nodes[parent->children[childIndex]];
  int newIndex = createNode(tree);
  BTreeNode *newNode = &tree->nodes[newIndex];
  newNode->isLeaf = child->isLeaf;
  newNode->numKeys = MAX_KEYS / 2;

  for (int j = 0; j < MAX_KEYS / 2; j++) {
    newNode->keys[j] = child->keys[j + MAX_KEYS / 2 + 1];
  }
  if (!child->isLeaf) {
    for (int j = 0; j < MAX_KEYS / 2 + 1; j++) {
      newNode->children[j] = child->children[j + MAX_KEYS / 2 + 1];
    }
  }
  child->numKeys = MAX_KEYS / 2;

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
