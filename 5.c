/*
 * Memory mapping the current/fastest implementation (2th.c)
 */
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define WORD_SIZE 256
#define MAX_ENTRIES 100000
#define HASHVAL 5381
#define err_abort(code, text)                                                  \
  do {                                                                         \
    fprintf(stderr, "%s at \"%s\":%d:%s\n", text, __FILE__, __LINE__,          \
            strerror(code));                                                   \
    abort();                                                                   \
  } while (0)

typedef struct node {
  char name[WORD_SIZE];
  int count;
  float min, max, sum;
  struct node *left;
  struct node *right;
} node;
typedef struct {
  char key[WORD_SIZE];
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

struct node *find_node(HashEntry map[], char *name) {
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

int main(void) {
  char name[WORD_SIZE], temp_container[10];
  static HashEntry map[MAX_ENTRIES];
  node *r = NULL, *n = NULL, *f = NULL;
  float temp;
  unsigned cur = 0;
  int fd;
  off_t size;
  char *addr, *end;

  fd = open("./measurements_1b.txt", O_RDONLY);
  if (fd == -1)
    err_abort(fd, "file open");

  size = lseek(fd, 0, SEEK_END);
  if (size == -1)
    err_abort(size, "lseek");

  addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (*addr == -1)
    err_abort(*addr, "mmap");
  printf("mmapped input file\n");

  end = addr + size;
  while (addr < end) {

    // reset containers for name and temp values
    memset(name, 0, WORD_SIZE);
    memset(temp_container, 0, 10);

    // hashname
    unsigned int hashval = 5381;
    unsigned int len = 0;
    while (*addr != ';') {
      hashval = ((hashval << 5) + hashval) + *addr;
      name[len++] = *addr++;
    }

    // advance past ;
    addr++;

    // reset length var to use for temperature value
    len = 0;

    // get temperature
    while (*addr != '\n') {
      temp_container[len++] = *addr++;
    }
    temp = atof(temp_container);

    // Skip past endline
    addr++;

    if ((f = find_node(map, name)) != NULL) {
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
  close(fd);
  return 0;
}
