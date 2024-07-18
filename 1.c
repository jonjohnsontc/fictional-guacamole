/*
    My first attempt at making a baseline calculation script in C
    This runs SLOW, it set my desktop PC to a crawl over the course of 20 mins 
    (i think, i didn't finish running it)

*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CITIES_MAX 43690
#define BUF_SIZE 4096
#define TEMP_SIZE 65536
#define WORD_SIZE 256

typedef struct measures {
  double min;
  double max;
  double mean;
  long count;
} measures;

typedef struct temp_array {
  size_t size;
  size_t total;
  float *values;
} temp_array;
// Holds representations of cities in the hashtable
typedef struct city_node {
  char name[WORD_SIZE];
  size_t total;
  struct city_node *next;
  struct temp_array *temps;
} city_node;
typedef temp_array *city_mapping[CITIES_MAX];
typedef city_node *city_names[CITIES_MAX];
// djb2 hash function - found via http://www.cse.yorku.ca/~oz/hash.html
// (https://stackoverflow.com/questions/7666509/hash-function-for-string)
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash;
}

// based off of in_hash_table setup
city_node *get_city(city_names cities, char *name) {
  unsigned code = hash(name) % CITIES_MAX;
  city_node *cp;
  cp = cities[code];
  while (cp) {
    if (strcmp(cp->name, name) == 0)
      return cp;
    cp = cp->next;
  }
  return NULL;
}

int add_measurement(city_names cns, char *name, float value, unsigned *id) {
  unsigned code;
  city_node *city;
  city = get_city(cns, name);
  if (!city) {
    code = hash(name) % CITIES_MAX;
    city = malloc(sizeof(city_node));
    if (city == NULL) {
      fprintf(stderr, "malloc error.\n");
      exit(1);
    }
    strcpy(city->name, name);
    city->total = 0;
    city->next = cns[code];
    cns[code] = city;
    
    city->temps = malloc(sizeof(temp_array));
    if (city->temps == NULL) {
      fprintf(stderr, "malloc error.\n");
      exit(1);
    }
    city->temps->size = 0;
    city->temps->total = BUF_SIZE;
    city->temps->values = (float *)malloc(city->temps->total * sizeof(float));
    city->temps->values[city->temps->size++] = value;
  } else {
    if (city->temps->size >= city->temps->total) {
      city->temps->total += BUF_SIZE;
      float *vals = realloc(city->temps->values, city->temps->total * sizeof(float));
      if (vals == NULL) {
        fprintf(stderr, "malloc error.\n");
        exit(1);
      }
      city->temps->values = vals;
    }
    city->temps->values[city->temps->size++] = value;
  }
  city->total++;
  return 0;
}

int main(void) {
  // let's test reading lines at a time in a cities file
  city_names NAMES = {NULL};
  unsigned cid;
  unsigned long iterations;
  float temp;
  char buf[BUF_SIZE];
  char name[WORD_SIZE];
  FILE *file = fopen("./measurements.txt", "r");
  iterations = 0;
  while (fgets(buf, sizeof(buf), file) != NULL) {
    sscanf(buf, "%[^;];%f", name, &temp);
    add_measurement(NAMES, name, temp, &cid);
    iterations += 1;
  }
  // assert(iterations == 10000); // iterations should match lines
  printf("Finished in %ld rounds\n", iterations);
  return 0;
}