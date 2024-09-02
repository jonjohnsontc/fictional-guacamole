#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline int cmp(const void *ptr_a, const void *ptr_b) {
  return strcoll((char *)ptr_a, (char *)ptr_b);
}

int main(void) {
  char *res = setlocale(LC_ALL, "en_US.utf8");
  if (res == NULL) {
    printf("Could not set locale\n");
  } else 
    printf("%s\n", res);

  char *words[6] = {"İstanbul", "Canyon", "fish", "xylophone", "Zeppelin", "ape"};
  qsort(words, 6, sizeof(char *), cmp);
  for (int i = 0; i < 6; i++) 
    printf("%d %s\n", i,words[i]);
  printf("\n");
}