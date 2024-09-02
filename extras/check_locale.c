#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline int collate(const void *ptr_a, const void *ptr_b) {
  return strcoll((char *)ptr_b, (char *)ptr_a);
}

static inline int cmp(const void *ptr_a, const void *ptr_b) {
  return strcmp((char *)ptr_b, (char *)ptr_a);
}

int main(void) {
  char *res = setlocale(LC_COLLATE, "en_US.UTF-8");
  if (res == NULL) {
    printf("Could not set locale\n");
  } else
    printf("%s\n", res);

  char *words[6] = {"İstanbul",  "Canyon",   "Fish",
                    "Xylophone", "Zeppelin", "Ape"};
  qsort(words, 6, sizeof(char *), cmp);
  for (int i = 0; i < 6; i++)
    printf("%d %s\n", i, words[i]);
  printf("\n");
}